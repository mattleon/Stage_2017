import sys
import os
import time
import subprocess
from os import popen

class Solver:
	nbProc = 0
	Proc = []
	running = 0
	
	def __init__(self, nbProc):
		self.nbProc = nbProc
		
	def runSolver(self, cmd):
		print(cmd)
		print("running...")
		self.running = 1
		FNULL = open(os.devnull, 'w')
		for i in range(self.nbProc):
			myproc = subprocess.Popen(cmd, stdout=FNULL, stderr=subprocess.STDOUT)
			self.Proc.append(myproc)
			
	def count_word(self, line):
		if line == "":
			return 0
		i = 0
		word = 1
		while i < len(line):
			if line[i] == ' ':
				word += 1
			if line[i] == '\0':
				word += 1
				return word
			if line[i] == '\n':
				word += 1
				return word
			i += 1
		return word
		
	def parse_line(self, line):
		nb_word = self.count_word(line)
		
		if nb_word == 1:
			if line == "kill":
				self.killall()
			elif line == "exit":
				self.stopPrg()
			else:
				print("commande inconnue")
		else:
			for i in range(len(line)):
				if line[i] == ' ':
					if line[:i] == "run":
						print(line[i+1:])
						if line[i+1:i+3] == "-p":
							print("-p")
							print("test: ", int(line[i+4]))
							self.nbProc = int(line[i+4])
							#self.runSolver(line[i+5)
							break
						else:
							#self.runSolver(line[i+1:)
							break
						
	def view(self):
		print("view..")

	def run(self, arg):
		if self.running == 1:
			print("Already running.")
			exit
		if self.Proc.__len__() > 0:
			for i in range(len(self.Proc)):
				del Proc[0]
		runSolver(arg)
		
	def killall(self):
		print("killing all process...")
		if self.Proc.__len__() != 0 and self.running == 1:	
			for i in range(len(self.Proc)):
				self.Proc[i].kill()
				print("Processus kill")
			self.running = 0
			
	def stopPrg(self):
		self.killall()
		print("exiting..")
		exit()
		
	def inter(self):
		while True:
			cmd = input()
			self.parse_line(cmd)
				
if __name__ == '__main__':
	if len(sys.argv) > 1:
		if sys.argv[1] == "-p":
			solver = Solver(int(sys.argv[2]))
			#solver.runSolver(sys.argv[3:])
		else:
			#on recupere nombre coeur
			f = popen('if [ `uname` = "Darwin" ]; then sysctl machdep.cpu.thread_count | sed "s/.* //"; else grep processor /proc/cpuinfo | wc -l; fi')
			solver = Solver(int(f.read()))
			#solver.runSolver(sys.argv[1:])
		solver.inter()
	else:
		print(sys.argv[0], " [-p <nbprocs>] <batch_args>")
		print("if -p is not given, it is read from /proc/cpuinfo")
