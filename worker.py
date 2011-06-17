#!/usr/bin/python3
# -*- coding: utf-8 -*-

import argparse, sys, subprocess, xmlrpc.client, signal

class InterruptedError(Exception):
	pass

def execute(pool,  pool_args,  worker,  worker_args,  worker_count,  hub,  machine, force=False):
	dcs = xmlrpc.client.ServerProxy(hub)
	if pool_args==None:
		pool_args=[]
	if worker_args==None:
		worker_args=[]
	try:
		if force:
			try:
				dcs.remove_machine(machine)
			except:
				print("Unknown error: {0}".format(sys.exc_info()))
		dcs.add_machine(machine,  "0")
		#with subprocess.Popen([pool]+pool_args,  executable=pool,  stdout=sys.stdout,  stderr=sys.stderr) as p:
		with subprocess.Popen([pool]+pool_args,  executable=pool,  stdout=subprocess.PIPE,  stderr=subprocess.STDOUT) as p:
			w = []
			try:
				for i in range(worker_count):
					w += [subprocess.Popen([worker]+worker_args,  executable=worker,  stdout=sys.stdout,  stderr=sys.stderr)]
				while p.poll()==None:
					print(p.stdout.readline().decode('utf8'),  end='')
			finally:
				for i in w:
					try:
						i.terminate()
						i.wait()
					except:
						print("Unknown error: {0}".format(sys.exc_info()))
				try:
					p.terminate()
					p.wait()
				except:
					print("Unknown error: {0}".format(sys.exc_info()))
	except (InterruptedError, KeyboardInterrupt) as e:
		print("Execution was interrupted by {0}".format(e))
		raise
	except:
		print("Unknown error: {0}".format(sys.exc_info()))
	finally:
		dcs.remove_machine(machine)

def exceptionRaiser(signum, frame):
	raise(InterruptedError(signum))

if __name__=='__main__':
	signal.signal(signal.SIGTERM, exceptionRaiser)
	parser = argparse.ArgumentParser("Worker starter")
	parser.add_argument('-v', '--version', action='version', version='%(prog)s 0.0.1', help="version information")
	parser.add_argument('-p',  '--pool',  action='store',  dest='pool',  help='pool binary', required=True)
	parser.add_argument('-w',  '--worker',  action='store',  dest='worker',  help='worker binary', required=True)
	parser.add_argument('--pool-args',  action='store',  dest='pool_args',  help='pool args')
	parser.add_argument('--worker-args',  action='store',  dest='worker_args',  help='worker args')
	parser.add_argument('-c', '--worker-count',  action='store',  dest='worker_count',  type=int,  help='worker count',  required=True)
	parser.add_argument('-d', '--hub',  action='store',  dest='hub',  help='hub xmlrpc interface',  required=True)
	parser.add_argument('-m', '--machine',  action='store',  dest='machine',  help='machine name',  required=True)
	parser.add_argument('-r', '--restart',  action='store_true',  dest='restart',  help='auto restart execution except keyboard interruption and SIGTERM')
	args = parser.parse_args()
	while True:
		execute(args.pool,  args.pool_args,  args.worker,  args.worker_args,  args.worker_count,  args.hub,  args.machine)
		if not args.restart:
			break
		else:
			print("Restarting", file=sys.stderr)
