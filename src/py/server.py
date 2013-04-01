#!/usr/bin/env python2.7
from __future__ import print_function

import asyncore
import asynchat
import socket
import BaseHTTPServer
import urllib
import urlparse
import os
import os.path
import logging
import logging.handlers
import sys
import collections
import subprocess
import Queue
import threading
import time
import select
import cStringIO

class BadUrlException(Exception): pass

class GameRequestHandler(asynchat.async_chat, BaseHTTPServer.BaseHTTPRequestHandler):

    def __init__(self, sock, address, server):
        self.client_address = address
        self.connection = sock
        asynchat.async_chat.__init__(self, sock=sock)
        self.server = server

        self.set_terminator('\r\n\r\n')

        self.in_buffer = []

    def collect_incoming_data(self, data):
        self.in_buffer.append(data)

    def found_terminator(self):
        self.rfile = cStringIO.StringIO(''.join(self.in_buffer))
        self.rfile.seek(0)
        self.wfile = cStringIO.StringIO()
        self.raw_requestline = self.rfile.readline()
        self.parse_request()
        if self.command == 'GET':
            self.do_GET()

    def do_GET(self):
        unquoted = urllib.unquote(self.path)
        parsed = urlparse.urlparse(unquoted)
        self.parsed_path = parsed
        path = parsed.path.split('/')
        command = path[-1]
        game_name = path[-2]

        # Can't use urlparse.parse_qs because of equal signs in board string
        query = collections.defaultdict(str)
        query_lst = parsed.query.split('&')
        for t in query_lst:
            if t != '':
                p = t.split('=', 1)
                query[p[0]] = p[1]

        self.server.log.info('GET: {}'.format(unquoted))

        game = self.server.get_game(game_name)
        c_command = {
            'getStart' : 'start_response',
            'getNextMoveValues':
            'next_move_values_response {}'.format(query['board']),
            'getMoveValue': 'move_value_response {}'.format(query['board'])
        }[command]
        game.push_request(GameRequest(self, query, c_command))

    def respond(self, response):
        self.send_header('Content-Length', len(response))
        self.send_header('Content-Type', 'text/plain')
        self.send_header('Access-Control-Allow-Origin', '*')
        self.end_headers()
        self.push(response)
        self.close_when_done()


class GameRequestServer(asyncore.dispatcher):

    def __init__(self, address, handler, log=logging.getLogger("server")):
        self.ip, self.port = address
        self.log = log
        self.handler = handler
        self.log.info('Starting server on port {}.'.format(self.port))
        self._game_table = {}
        asyncore.dispatcher.__init__(self)

        self.create_socket(socket.AF_INET, socket.SOCK_STREAM)

        self.set_reuse_addr()

        self.bind(address)

        self.listen(4096)

    def get_game(self, name):
        try:
            return self._game_table[name]
        except KeyError:
            return self.load_game(name)

    def load_game(self, name):
        game = Game(self, name)
        self._game_table[name] = game
        return game

    def serve_forever(self):
        asyncore.loop(0.01)

    def handle_accept(self):
        try:
            connection, address = self.accept()
        except socket.error:
            self.log.error('Server accept() failed.')
        else:
            self.handler(connection, address, self)


class GameRequest(object):

    def __init__(self, handler, query, command):
        self.handler = handler
        self.query = query
        self.command = command

class GameProcessRequest(object):

    def __init__(self, handler, command):
        self.command = command
        self.handler = handler

    def respond(self, response):
        self.handler.respond(response)


class GameProcess(object):

    def __init__(self, server, game, bin_path, option_num=None):
        self.server = server
        self.game = game
        self.queue = Queue.Queue()
        self.option_num = option_num
        # Number of seconds to wait without a request before shutting down
        # process
        self.req_timeout = 10
        # Seconds to wait for a response from the process
        self.read_timeout = 5

        self.timeout_error = 'Game subprocess failed.'

        self.alive = True

        # Note that arguments to GamesmanClassic must be given in the right
        # order (this one, to be precise).
        arg_list = [bin_path] 
        if option_num is not None:
            arg_list.append('--option={}'.format(option_num))
        #arg_list.append('--notiers')
        arg_list.append('--interact')

        # Open a subprocess, connecting all of its file descriptors to pipes,
        # and set it to line buffer mode.
        self.process = subprocess.Popen(arg_list, stdin=subprocess.PIPE,
                stdout=subprocess.PIPE, stderr=subprocess.PIPE, bufsize=1,
                close_fds=True)
        self.thread = threading.Thread(target=self.request_loop)
        self.thread.daemon = True
        self.thread.start()

    def check_alive(self):
        assert self.alive, "Zombie thread in GameProcess."

    def push_request(self, request):
        self.queue.put(request)

    def close(self):
        self.check_alive()
        self.alive = False
        self.server.log.info('Closing {}.'.format(self))
        self.process.terminate()
        time.sleep(1)
        self.process.kill()
        self.game.remove_process(self)

    def is_response(self, response):
        return 'result' in response

    def parse_response(self, response):
        lines = response.split('\n')
        for line in lines:
            if line.startswith('result'):
                result = line.split('=>>')[1].strip()
                self.server.log.info('Parsed response to {}.'.format(result))
                return result


    def request_loop(self):
        self.check_alive()
        live = True
        while live:
            self.check_alive()
            try:
                request = self.queue.get(block=True, timeout=self.req_timeout)
            except Queue.Empty as e:
                self.server.log.error(
                    '{} closed from lack of use.'.format(self.game.name))
                live = False
            else:
                self.process.stdin.write(request.command + '\n')
                response = ''
                timeout = 0.01
                count = 0
                while True:
                    count += 1
                    if count * timeout >= self.read_timeout:
                        # Did not receive a response from the subprocess, so
                        # exit and kill it.
                        self.server.log.error('Did not receive result'
                            'from {}!'.format(self.game.name))
                        live = False
                        break
                    rlist, _, _ = select.select([self.process.stdout], [], [], timeout)
                    response += self.process.stdout.readline()
                    if self.is_response(response):
                        parsed = self.parse_response(response)
                        self.server.log.info('Send response: {}'.format(parsed))
                        request.respond(parsed)
                        break
        while not self.queue.empty():
            self.queue.get().respond(self.timeout_error)
        self.close()


class Game(object):

    def __init__(self, server, name):
        self.server = server
        self.name = name
        self.root_dir = root_game_directory
        self.processes = {}

    def get_process(self, query):
        opt = self.get_option(query)

        # different from (opt in self.processes)
        if self.processes.get(opt, False):
            return self.processes[opt][0]
        else:
            return self.start_process(query)

    def start_process(self, query):
        bin_name = 'm' + self.name
        for f in os.listdir(self.root_dir):
            if f == bin_name:
                self.server.log.info('Starting {}.'.format(self.name))
                bin_path = os.path.join(self.root_dir, bin_name)
                opt = self.get_option(query)
                gp = None
                try:
                    gp = GameProcess(self.server, self, bin_path, opt)
                except OSError as err:
                    self.server.log.error('Ran out of file descriptors!')
                else:
                    proc_list = self.processes.setdefault(opt, [])
                    proc_list.append(gp)
                return gp

    def push_request(self, request):
        proc = self.get_process(request.query)
        if proc:
            proc.push_request(GameProcessRequest(request.handler, request.command))

    def get_option(self, query):
        return None

    def remove_process(self, process):
        self.server.log.info('Removing {}.'.format(process))
        try:
            self.processes[process.option_num].remove(process)
        except ValueError as e:
            self.server.log.error('Trying to remove subprocess of {} failed \
            because it could not be found.'.format(self.name))



port = 8081
root_game_directory = './bin/'
log_filename = 'server.log'
max_log_size = 10 * 1024 **2


def get_log():
    log = logging.getLogger('server')

    file_logger = logging.handlers.RotatingFileHandler(log_filename,
        maxBytes=max_log_size)
    file_logger.setLevel(logging.DEBUG)
    log.addHandler(file_logger)

    stdout_logger = logging.StreamHandler(sys.stdout)
    stdout_logger.setLevel(logging.DEBUG)
    log.addHandler(file_logger)

    log.setLevel(logging.DEBUG)
    return log


def main():
    httpd = GameRequestServer(('', port), GameRequestHandler, log=get_log())
    httpd.serve_forever()


if __name__ == '__main__':
    main()