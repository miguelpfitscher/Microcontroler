whichWorkload = 1
row = 3
col = 4
workload = [[0 for x in range (col)] for x in range (row)]


outputTable = []

if whichWorkload == 0: #workload 0
	workload = [
	[25, 25, 8, 0],
	[50, 50, 13, 0],
	[100, 100, 40, 0]]
elif whichWorkload == 1: #workload 1
	workload = [
	[20, 20, 5, 1], # 1ms self suspension
	[30, 30, 12, 0],
	[50, 50, 15, 0]]
elif whichWorkload == 2:
	workload = [
	[25, 25, 5, 0],
	[30, 23, 12, 0],
	[50, 45, 15, 0]]
else:
	workload = [
	[25, 25, 8, 0],
	[50, 50, 13, 2],
	[100, 100, 40, 2]]


def timeAnalysis():
	T = []
	Schedulable = []
	for k in range (row):
		T = []
		e = 0
		for i in range (k + 1): # t0
			e += workload[i][2] # e
		T.append(e)
		while T[len(T) - 1] < workload[k][1]:
			t = int(T[len(T)-1])
			wi = 0
			for j in range (k):
				wi += t * float(workload[j][2])/float(workload[j][0]) # sum j =0 to k -1

			wi+= workload[k][2] + workload[k][3] # wi = ei + b+ sum
			T.append(wi)
			if T[len(T)-1] == T[len(T)-2]:
				Schedulable.append(k)
				break
		print T #Tk

	if (len(Schedulable) == row):
 		print "T is Schedulable"
	else:
		print "T is not Schedulable"

print "WorkLoad: " + str(workload)
timeAnalysis()
