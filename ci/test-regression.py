#!/usr/bin/env python3

# =============================================================================
# ==================================================================== PACKAGES

import os
import sys
import math
import time
import pathlib
import argparse
import subprocess

# ==================================================================== PACKAGES
# =============================================================================

# =============================================================================
# ================================================================== PARAMETERS

parser = argparse.ArgumentParser(prog='aff3ct-test-regression', formatter_class=argparse.ArgumentDefaultsHelpFormatter)
parser.add_argument('--refs-path',      action='store', dest='refsPath',      type=str,   default="refs",                    help='Path to the references to re-simulate.')
parser.add_argument('--results-path',   action='store', dest='resultsPath',   type=str,   default="test-regression-results", help='Path to the simulated results.')
parser.add_argument('--build-path',     action='store', dest='buildPath',     type=str,   default="build",                   help='Path to the AFF3CT build.')
parser.add_argument('--start-id',       action='store', dest='startId',       type=int,   default=1,                         help='Starting id to avoid computing results one again.')                                     # choices=xrange(1,   +inf)
parser.add_argument('--sensibility',    action='store', dest='sensibility',   type=float, default=1.0,                       help='Sensibility to verify a SNR point.')                                                    # choices=xrange(0.0, +inf) 
parser.add_argument('--n-threads',      action='store', dest='nThreads',      type=int,   default=0,                         help='Number of threads to use in the simulation (0 = all available).')                       # choices=xrange(0,   +ing)
parser.add_argument('--recursive-scan', action='store', dest='recursiveScan', type=bool,  default=True,                      help='If enabled, scan the path of refs recursively.')
parser.add_argument('--max-fe',         action='store', dest='maxFE',         type=int,   default=100,                       help='Maximum number of frames errors to simulate per SNR point.')                            # choices=xrange(0,   +inf)
parser.add_argument('--weak-rate',      action='store', dest='weakRate',      type=float, default=0.8,                       help='Rate of valid SNR points to passed a test.')                                            # choices=xrange(0.0, 1.0 )
parser.add_argument('--max-snr-time',   action='store', dest='maxSNRTime',    type=int,   default=600,                       help='The maximum amount of time to spend to compute a SNR point in seconds (0 = illimited)') # choices=xrange(0,   +inf)
parser.add_argument('--verbose',        action='store', dest='verbose',       type=bool,  default=False,                     help='Enable the verbose mode.')

# supported file extensions (filename suffix)
extensions = ['.txt', '.perf', '.data', '.dat']

# ================================================================== PARAMETERS
# =============================================================================

# =============================================================================
# =================================================================== FUNCTIONS

def getFileNames(currentPath, fileNames):
	if os.path.isdir(currentPath):
		if not os.path.exists(currentPath.replace(args.refsPath, args.resultsPath)):
			os.makedirs(currentPath.replace(args.refsPath, args.resultsPath))

		files = os.listdir(currentPath)
		for f in files:
			if "~" in f:
				continue
			newCurrentPath = currentPath + "/" + f
			if os.path.isdir(newCurrentPath):
				if args.recursiveScan:
					getFileNames(newCurrentPath, fileNames)
			else:
				getFileNames(newCurrentPath, fileNames)
	elif os.path.isfile(currentPath):
		if pathlib.Path(currentPath).suffix in extensions:
			if args.refsPath == currentPath:
				basename = os.path.basename(args.refsPath)
				dirname = args.refsPath.replace(basename, '')
				args.refsPath = dirname
				fileNames.append(basename)
			else:
				shortenPath = currentPath.replace(args.refsPath + "/", "")
				shortenPath = shortenPath.replace(args.refsPath,       "")
				fileNames.append(shortenPath)
	else:
		print("# (WW) The path '", currentPath, "' does not exist.")

# -----

# =================================================================== FUNCTIONS
# =============================================================================

# =============================================================================
# ======================================================================== MAIN

#parser.print_help()
args = parser.parse_args()

if args.startId <= 0:
	args.startId = 1

print('# AFF3CT tests')
print('# ------------')
print('#')
print('# Parameters:')
print('# refs path      =', args.refsPath     )
print('# results path   =', args.resultsPath  )
print('# build path     =', args.buildPath    )
print('# start id       =', args.startId      )
print('# sensibility    =', args.sensibility  )
print('# n threads      =', args.nThreads     )
print('# recursive scan =', args.recursiveScan)
print('# max fe         =', args.maxFE        )
print('# weak rate      =', args.weakRate     )
print('# max snr time   =', args.maxSNRTime   )
print('# verbose        =', args.verbose      )
print('#')

PathOrigin = os.getcwd()

if os.path.isfile(args.resultsPath):
	print("# (EE) The results path should not be an existing file.")
	sys.exit(1);

# auto create the result folder if it does not exist
if not os.path.exists(args.resultsPath):
	os.makedirs(args.resultsPath)

# get the filenames to test
fileNames = []
getFileNames(args.refsPath, fileNames)

if (len(fileNames) - (args.startId -1) > 0) :
	print("# (II) Starting the test script...")
else:
	print("# (WW) There is no simulation to replay.")

failIds = []
nErrors = 0
testId = 0
for fn in fileNames:
	if testId < args.startId -1:
		testId = testId + 1
		continue

	print("Test n°" + str(testId+1) + " / " + str(len(fileNames)) + 
	      " - " + fn, end="", flush=True);

	# open the file in read mode (from the fileName "fn" and the path)
	f = open(args.refsPath + "/" + fn, 'r')

	# read all the lines from the current file f
	lines = []
	for line in f:
		lines.append(line)

	# get the main infos from the lines
	runCommand = lines[1].strip()

	currentSection = ""
	idx = 0
	simuRef = []
	for l in lines:
		# avoid the first lines and the comments
		if idx > 6 and l.replace(" ", "") != ""   and \
		               l.replace(" ", "") != "\n" and \
		               l[0] != '#':
			simuRef.append(l.strip().replace("||", "|").replace(" ", "").split("|"))
		idx = idx +1

	f.close()

	# split the run command
	argsAFFECT = [""]
	idx = 0

	new = 0
	found_dashes = 0
	for s in runCommand:
		if found_dashes == 0:
			if s == ' ':
				if new == 0:
					argsAFFECT.append("")
					idx = idx + 1
					new = 1

			elif s == '\"':
				if new == 0:
					argsAFFECT.append("")
					idx = idx + 1
					new = 1
				found_dashes = 1

			else:
				argsAFFECT[idx] += s
				new = 0

		else: # between dashes
			if s == '\"':
				argsAFFECT.append("")
				idx = idx + 1
				found_dashes = 0

			else:
				argsAFFECT[idx] += s

	del argsAFFECT[idx]

	argsAFFECT.append("--ter-freq")
	argsAFFECT.append("0")
	if args.maxFE:
		argsAFFECT.append("-e")
		argsAFFECT.append(str(args.maxFE))
	argsAFFECT.append("-t")
	argsAFFECT.append(str(args.nThreads))
	argsAFFECT.append("--sim-no-colors")
	if args.maxSNRTime:
		argsAFFECT.append("--sim-stop-time")
		argsAFFECT.append(str(args.maxSNRTime))

	os.chdir(args.buildPath)
	startTime = time.time()
	processAFFECT = subprocess.Popen(argsAFFECT, stdout=subprocess.PIPE, 
	                                             stderr=subprocess.PIPE)
	(stdoutAFFECT, stderrAFFECT) = processAFFECT.communicate()
	elapsedTime = time.time() - startTime

	err = stderrAFFECT.decode(encoding='UTF-8')
	if err:
		print(" - ABORTED.", end="\n");
		print("Error message:", end="\n");
		print(err)
		nErrors = nErrors +1
		failIds.append(testId +1)
	else:
		# begin to write the results into a file
		os.chdir(PathOrigin)

		fRes = open(args.resultsPath + "/" + fn, 'w+')

		# parse the results
		stdOutput = stdoutAFFECT.decode(encoding='UTF-8').split("\n")
		outputAFFECTLines = []
		simuCur = []
		for l in stdOutput:
			if l != "" and l[0] != '#' and l[0] != "(": # avoid the first lines and the comments
				array = l.strip().replace("||", "|").replace(" ", "").split("|")
				if (len(array) == 12 or len(array) == 9):
					outputAFFECTLines.append(l)
					simuCur.append(array)
				else:
					fRes.write(l + "\n")
			elif l != "" and l[0] == '#' and "# End of the simulation." not in l:
				fRes.write(l + "\n")

		fRes.flush()

		# validate (or not) the BER/FER performance
		valid = 0
		idx = 0
		errorsList = []
		sensibilityList = []
		for ref in simuRef:
			try:
				cur_fe = int(simuCur[idx][4])
			except IndexError: # no such line
				break

			if cur_fe < args.maxFE / 2:
				break

			numRef = float(ref[6][0:4])
			powerRef = int(ref[6][6:8])
			numCur = float(simuCur[idx][6][0:4])
			powerCur = int(simuCur[idx][6][6:8])

			if powerRef - powerCur != 0:
				if powerRef > powerCur:
					numCur = numCur * 10**(math.fabs(powerCur - powerRef))
				else:
					numRef = numRef * 10**(math.fabs(powerCur - powerRef))

			absoluteNumDiff = math.fabs(numCur - numRef)
			sensibilityList.append(absoluteNumDiff)
			if absoluteNumDiff > args.sensibility:
				fRes.write(outputAFFECTLines[idx] + "WRONG! FER=" + ref[6][0:8] + "\n")
				errorsList.append([float(simuCur[idx][1][0:4]), numCur - numRef])
			else:
				valid = valid + 1
				fRes.write(outputAFFECTLines[idx] + "\n")

			idx = idx + 1

		print(" - %.2f" %elapsedTime, "sec", end="")
		if valid == idx:
			print(" - STRONG PASSED.", end="\n");
		elif idx != 0 and float(valid) / float(idx) >= args.weakRate:
			print(" - WEAK PASSED.", end="\n");
		else:
			print(" - FAILED.", end="\n");
			nErrors = nErrors +1
			failIds.append(testId +1)

		if args.verbose:
			avgSensibility = 0
			minSensibility = 0
			maxSensibility = 0
			if idx == 0:
				avgSensibility = 0
			else:
				avgSensibility = sum(sensibilityList) / float(idx)
				minSensibility = min(sensibilityList)
				maxSensibility = max(sensibilityList)
			rateSensibility = (avgSensibility / args.sensibility) * 100

			print("---- Details: 'valid SNR points' = ", valid, "/", idx, 
			      ", 'sensibility [avg,min,max,rate]' = [ %.2f" %avgSensibility, 
			      ", %.2f" %minSensibility, ", %.2f" %maxSensibility, 
			      ", %.1f" % rateSensibility, "% ].", end="\n")
			if idx > 0:
				print("---- Details: 'first SNR point' =", float(simuCur[0][1][0:4]), 
				      "dB (@", simuCur[0][6][0:8], 
				      "FER), 'last SNR point' =", float(simuCur[idx -1][1][0:4]), 
				      "dB (@", simuCur[idx -1][6][0:8], "FER).")
			if len(errorsList):
				print("---- Details: 'errors list' = [", end="")
				el = 0
				for error in errorsList:
					if error[1] > 0:
						print("{", error[0], "dB -> +%.2f" %error[1], "}", end="")
					else:
						print("{", error[0], "dB -> %.2f" %error[1], "}", end="")
					if el < len(errorsList) -1:
						print(", ", end="")
					el = el + 1
				print("].", end="\n")

		fRes.write("# End of the simulation.\n")
		fRes.close();

	testId = testId + 1


if len(fileNames) - (args.startId -1) > 0:
	if nErrors == 0:
		print("# (II) All the tests PASSED !", end="\n");
	else:
		print("# (II) Some tests FAILED: ", end="")
		f = 0
		for failId in failIds:
			print("n°", end="")
			print(str(failId), end="")
			if f == len(failIds) -1:
				print(".", end="\n")
			else:
				print(", ", end="")
			f = f + 1

sys.exit(nErrors);

# ======================================================================== MAIN
# =============================================================================
