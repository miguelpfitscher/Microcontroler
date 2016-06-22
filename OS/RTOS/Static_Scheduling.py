
whichWorkload = 0
row = 8
col = 6
workload = [[0 for x in range (col)] for x in range (row)]


if whichWorkload: #workload 2
	hyperPeriod = 200
	workload = [
	[20, 4, 0, 15, 0, 15],
	[20, 1, 5, 20, 5, 20],
	[30, 2, 5, 30, 5, 30],
	[30, 1, 5, 30, 5, 30],
	[50, 1, 10, 40, 10, 40],
	[50, 1, 10, 40, 10, 40],
	[50, 2, 25, 50, 25, 50],
	[50, 5, 25, 50, 25, 50]]
else: #workload 1
	hyperPeriod = 160
	workload = [
	[10, 2, 0, 10, 0, 10],
	[10, 2, 0, 10, 0, 10],
	[20, 1, 5, 20, 5, 20],
	[20, 2, 5, 20, 5, 20],
	[40, 2, 5, 30, 5, 30],
	[40, 2, 5, 30, 5, 30],
	[80, 2, 10, 60, 10, 60],
	[80, 2, 10, 60, 10, 60]]

outputTable = []

def verifyMiss():
	deadList=[]
	deadLineMiss=[]
	for i in range (row):
		deadList.append(workload[i][3]+workload[i][2])

	for i in range (len(outputTable)):
		if(deadList[outputTable[i][0]] < outputTable[i][1]):
			deadLineMiss.append("Job: "+str(outputTable[i][0])+". Processed Time: "+str(outputTable[i][1])+". DeadLine: "+ str(deadList[outputTable[i][0]]))
			deadList[outputTable[i][0]]+=workload[outputTable[i][0]][0]
		else:
			deadList[outputTable[i][0]]+=workload[outputTable[i][0]][0]

	print "Workload: "+ str(whichWorkload + 1)
	print outputTable
	print (deadLineMiss)
	print "Number of Misses: " + str(len(deadLineMiss))


def priorityTest(queue):
	highPriority = 0
	for i in range (len(queue)):
		if(workload[queue[i]][5] + workload[queue[i]][2] < workload[queue[highPriority]][5]+ workload[queue[highPriority]][2]):
			highPriority = i

	return highPriority

def scheduling():
	inProcessor = []
	readyQueue = []
	time = 0

	while time < hyperPeriod:

		for i in range (row):
			if workload [i][2] <= time and workload[i][4] <= time:
				try:
					readyQueue.index(i)
				except ValueError:
					readyQueue.append(i)

		if not len(inProcessor) and len(readyQueue):
			highPriority = priorityTest(readyQueue)
			#print "time and queue" + str(readyQueue) + str(time)
			i = readyQueue.pop(highPriority)
			c=[]
			c.append(i)
			c.append(time)
			c.append(workload[i][5]+workload[i][2])
			workload[i][4]+= workload[i][0]
			workload[i][5]+= workload[i][0]

			inProcessor = list(workload[i])



			inProcessor[1]+= time
			outputTable.append(list(c))
		elif len(inProcessor):
			time+=1
			if time >= inProcessor[1]:
				inProcessor = []
		else:
			time+=1


scheduling()
verifyMiss()
