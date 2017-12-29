from __future__ import division
import sys
import argparse
import re
import collections


#class for save the data from index.txt
class Func:

	def __init__(self,index,fname,file,line):
		self.index = index
		self.fname = fname
		self.file = file
		self.line = line

	def printFunc(self):
		print "Index: " + self.index
		print "Name: " + self.fname
		print "File: " + self.file
		print "Line: " + self.line

		pass

#Get Char key --> to int
def get_key(key):
    try:
        return int(key)
    except ValueError:
        return key


#Get the values to do the three rule to normalize results
def getValues(values):
	lines = values.splitlines()
	valorz = [[]*2 for j in xrange(2)]
	valorz[0] = map(int, lines[0].split())
	valorz[1] = map(int, lines[1].split())
	vals = []

	for i in range(len(valorz[0])):
		vals.append(valorz[0][i]/valorz[1][i])

	return vals

#will normalize the results
def changeResults(line, mainValues):
	valstr = ""
	values = map(int, line.split())

	for i in range(len(values)):
		valstr += str((int(values[i]*mainValues[i]))) + " "

	return valstr

#parse the result files
def parseResults(file_text, indexFuncs, mainValues):
	newText = []
	lines = file_text.splitlines()
	
	newText.append(lines[0])
	newText.append("\n#define function ID Mapping")

	funcsAux = collections.OrderedDict(sorted(indexFuncs.items(), key=lambda t: get_key(t[0])))

	for key in funcsAux:
		aux = "fn=("+indexFuncs[key].index+") "+ indexFuncs[key].fname + ":" + indexFuncs[key].line
		newText.append(aux)

	newText.append("\n")

	#find only the first lines
	my_reg = re.escape("fn=(") + r"[0-9]+"

	for i in range(len(lines)):
		if re.match(my_reg, lines[i]) is not None:
			gr = re.search("[0-9]+", lines[i])
			newText.append("fl= "+indexFuncs[gr.group(0)].file)
			newText.append(lines[i])
			lines[i+2] = changeResults(lines[i+2], mainValues)
			lines[i+2] = indexFuncs[gr.group(0)].line + " "+ lines[i+2]
			newText.append(lines[i+2])

	return newText

#parse the index file
def parseIndex(file_text, directory):
	functions = {}

	for line in file_text.splitlines():
		st = re.split(':', line)

		fn = Func(st[0],st[4], st[1], st[2])

		functions[st[0]] = fn

	return functions


#read the file
def writeFile(filename, text):
	f = open(filename, "w")

	for item in text:
		f.write("%s \n" % item)

	f.close()

	return f


#read the file
def readFile(filename):
	txt = open(filename)

	data = txt.read()

	txt.close()

	return data

#define the args for the progamme
def defineArgs():

	parser = argparse.ArgumentParser(description='Transforms the rapl output into callgrind format.')

	parser.add_argument('-i', dest='index', type=str, help='index file of the instrumentation')
	parser.add_argument('-f', dest='results', type=str, help='results of rapl measures')
	parser.add_argument('-d', dest='directory', type=str, help='directory of the executed programme')
	parser.add_argument('-o', dest='output', type=str, help='name of the output file to generate')
	parser.add_argument('-m', dest='main', type=str, help='file with the results with the main function normalized')
	

	return parser


def main(argv):

	print "Running the Script...\n"

	parser = defineArgs()

	args = parser.parse_args()

	mainValues = [1,1,1,1,1]

	#print args.index;
	#print args.results;
	#print args.directory;
	#print args.output;

	if(args.main):
		valuesMain = readFile(args.main)
		mainValues = getValues(valuesMain)

	textIndex = readFile(args.index)
	indexFuncs = parseIndex(textIndex, args.directory)
	print "Parse the Index: Done."

	textResults = readFile(args.results)
	kcText = parseResults(textResults, indexFuncs, mainValues)
	print "Parse the Results: Done."


	if(args.output):
		writeFile(args.output, kcText)
	else:
		writeFile("outCallgrind.txt", kcText)

	print "Generate Output: Done.\n"	

	print "All done."

	pass


if __name__ == "__main__":
    main(sys.argv)
