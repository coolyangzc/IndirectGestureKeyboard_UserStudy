fd = open('Questionnaire_Study3.csv', 'rb')

metric = ['Speed', 'Accuracy', 'Fatigue', 'Preference']

lines = fd.readlines()
cnt = [([([0] * 5) for i in range(4)]) for i in range(4)]
for line in lines[1:]:
    data = line.decode('utf-8').split(',')
    if data[1] == 'Normal':
        c = 0
    else:
        c = 1
    if data[2] == 'List':
        m = 0
    else:
        m = 1
    c = c * 2 + m
    for i in range(4):
        score = int(data[3 + i])
        cnt[i][c][score - 1] += 1

chartfd = open('Questionnaire_Study3_chart.csv', 'w')
chartfd.write('type,1,2,3,4,5\n')
layouts = ['Absolute&List', 'Absolute&Pie', 'G-Keyboard&List', 'G-Keyboard&Pie']
for i in range(4):
    for j in range(4):
        chartfd.write(metric[i] + "(" + layouts[j] + ")")
        for k in range(5):
            chartfd.write(',' + str(cnt[i][j][k]))
        chartfd.write('\n')


