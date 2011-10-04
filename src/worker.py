#!/usr/bin/python3
# -*- coding: utf-8 -*-

import argparse, sys, subprocess, xmlrpc.client, signal, time

class InterruptedError(Exception):
	pass

class SingleProcess(object):
	def __init__(self, executable, args, stdout=None, stderr=None):
		self._starter = lambda: subprocess.Popen([executable]+args, executable=executable, stdout=stdout, stderr=stderr)
		try:
			self._inst = self._starter()
		except:
			try:
				self._inst.terminate()
				self._inst.wait()
			except:
				pass
			raise
	def __enter__(self):
		return self
	def __exit__(self, type, value, traceback):
		self._inst.terminate()
		self._inst.wait()
	def restart_dead(self):
		if self._inst.poll()!=None:
			print("Restarting process", file=sys.stderr)
			self._inst = self._starter()
	def wait(self):
		return self._inst.wait()
	def poll(self):
		return self._inst.poll()
	def terminate(self):
		self._inst.terminate()

class ProcessArray(object):
	def __init__(self, executable, args, count, stdout=None, stderr=None):
		self._starter = lambda: subprocess.Popen([executable]+args, executable=executable, stdout=stdout, stderr=stderr)
		self._inst = []
		try:
			for i in range(count):
				proc = self._starter()
				self._inst.append(proc)
		except:
			for i in self._inst:
				try:
					i.terminate()
					i.wait()
				except:
					pass
			raise
	def __enter__(self):
		return self
	def __exit__(self, type, value, traceback):
		for i in self._inst:
			i.terminate()
			i.wait()
	def restart_dead(self):
		for i in range(len(self._inst)):
			if self._inst[i].poll()!=None:
				print("Restarting process #{0}".format(i), file=sys.stderr)
				self._inst[i] = self._starter()
	def wait_all(self):
		w = []
		for i in self._inst:
			w.append(i.wait())
		return w
	def terminate_all(self):
		for i in self._inst:
			i.terminate()

def execute(pool, pool_args, worker, worker_args, worker_count, hub, machine, quiet, log, force=False):
	dcs = xmlrpc.client.ServerProxy(hub)
	if pool_args==None:
		pool_args=[]
	if worker_args==None:
		worker_args=[]
	try:
		if force:
			try:
				dcs.remove_machine(machine)
			except Exception as e:
				print("Expected error: {0}".format(e))
		dcs.add_machine(machine, "0")
		#n = open("/dev/null")
		#with SingleProcess(pool, pool_args, n, n) as p:
		#	with ProcessArray(worker, worker_args, worker_count, n, n) as w:
		with SingleProcess(pool, pool_args, sys.stdout, sys.stderr) as p:
			with ProcessArray(worker, worker_args, worker_count, sys.stdout, sys.stderr) as w:
				while True:
					time.sleep(1)
					p.restart_dead()
					w.restart_dead()
	except (InterruptedError, KeyboardInterrupt) as e:
		print("Execution was interrupted by {0}".format(e))
		raise
	except Exception as e:
		print("Unknown error: {0}".format(e))
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
	parser.add_argument('-q', '--quiet',  action='store_true',  dest='quiet',  help='will output nothing from pool and workers')
	parser.add_argument('-l', '--log', action='store', dest='log', help='directory were logs will be placed')
	args = parser.parse_args()
	execute(args.pool,  args.pool_args,  args.worker,  args.worker_args,  args.worker_count,  args.hub,  args.machine, args.quiet, args.log)
