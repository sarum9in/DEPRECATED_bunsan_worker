#!/usr/bin/python3
# -*- coding: utf-8 -*-

import argparse,  sys,  subprocess,  xmlrpc.client

def execute(pool,  pool_args,  worker,  worker_args,  worker_count,  hub,  machine):
	dcs = xmlrpc.client.ServerProxy(hub)
	if pool_args==None:
		pool_args=[]
	if worker_args==None:
		worker_args=[]
	try:
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
	except:
		print("Unknown error: {0}".format(sys.exc_info()))
	finally:
		dcs.remove_machine(machine)


if __name__=='__main__':
	parser = argparse.ArgumentParser("Worker starter")
	parser.add_argument('-v', '--version', action='version', version='%(prog)s 0.0.1', help="version information")
	parser.add_argument('-p',  '--pool',  action='store',  dest='pool',  help='pool binary', required=True)
	parser.add_argument('-w',  '--worker',  action='store',  dest='worker',  help='worker binary', required=True)
	parser.add_argument('--pool-args',  action='store',  dest='pool_args',  help='pool args')
	parser.add_argument('--worker-args',  action='store',  dest='worker_args',  help='worker args')
	parser.add_argument('-c', '--worker-count',  action='store',  dest='worker_count',  type=int,  help='worker count',  required=True)
	parser.add_argument('-d', '--hub',  action='store',  dest='hub',  help='hub xmlrpc interface',  required=True)
	parser.add_argument('-m', '--machine',  action='store',  dest='machine',  help='machine name',  required=True)
	args = parser.parse_args()
	execute(args.pool,  args.pool_args,  args.worker,  args.worker_args,  args.worker_count,  args.hub,  args.machine)
