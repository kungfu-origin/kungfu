#!/usr/bin/python


import csv

csv.register_dialect('mdmaker', delimiter = ',', lineterminator = '\n')

# format is:
# Security,
# bid valid,bid qty,bid px,
# ask px,ask qty,ask valid,
# Timestamp,
# execSummary,Price,DisplayQuantity,AggressorSide(1-buy,2-sell)

timestampMSecond = 100000L
timestampSecond = 100000000L

# contains csv file wtih the md for a single inst
# controls access and decodes lines
class Feed:
	def __init__(self, filename):
		self.feed = csv.reader(open(filename, 'r'), dialect='mdmaker')
		self.line = []
		self.next()

	# get the next line from the csv file. returns True if successful
	def next(self):
		try:
			self.line = self.feed.next()
		except StopIteration:
			self.line = []
			return False
#		print "R %s" % self.line
		return self.valid()

	def valid(self):
		return len(self.line) > 0

	def getLine(self):
		return self.line;

	def modifyTime(self, newTime):
		self.line[7] = newTime

	# return the line, modifying the time such that its beyond the value specified
	def incrementTime(self, newTime):
#		print "checktime"
		if self.getTime() <= newTime:
#			print "inctime"
			self.modifyTime(newTime + timestampMSecond)
		return self

	def getTime(self):
		if len(self.line) == 0:
			return 0L
		else:
			return long(self.line[7])

	def invalidate(self):
		self.line = []
		
class Writer:
	def __init__(self, filename):
		afile = open(filename, 'w')
		afile.write("Security,bid valid,bid qty,bid px,ask px,ask qty,ask valid,Timestamp,execSummary,Price,DisplayQuantity,AggressorSide(1-buy,2-sell)\n")
		self.output = csv.writer(afile, dialect='mdmaker')
		self.lastTime = 0L

	def writeToMatchTimes(self, instA, instB):
		while instA.valid() and instA.getTime() < instB.getTime():
			self.write(instA.incrementTime(self.lastTime))
			instA.next()
		if instB.valid(): self.write(instB.incrementTime(self.lastTime))
		if instA.valid(): self.write(instA.incrementTime(self.lastTime))

	def merge(self, instA, instB):
		# ensure both timestamps exceed the last written
		if instA.getTime() == self.lastTime:
			instA.modifyTime(self.lastTime + timestampMSecond)
		if instB.getTime() == self.lastTime:
			instB.modifyTime(self.lastTime + timestampMSecond)

		if instA.getTime() > instB.getTime():
#			print "instA ahead instB"
			self.writeToMatchTimes(instB, instA)
		elif instB.getTime() > instA.getTime():
#			print "instB ahead instA"
			self.writeToMatchTimes(instA, instB)
		else: # equal
#			print "match"
			self.write(instA.incrementTime(self.lastTime))
			self.write(instB.incrementTime(self.lastTime))

	def write(self, feed):
		self.output.writerow(feed.getLine())
#		print "W %s" % feed.getLine()
		self.lastTime = feed.getTime()

def readInput(instA, instB):
	instAValid = instA.next()
	instBValid = instB.next()
	return instAValid or instBValid



a = Feed("A.csv")
b = Feed("B.csv")
writer = Writer("merged.csv")

while readInput(a, b):
	if a.valid() and b.valid():
		writer.merge(a, b)
	elif a.valid():
		writer.write(a)
	elif b.valid():
		writer.write(b)

