fd = open('Questionnaire_Study1.csv', 'rb')
outfd = open('Questionnaire_Study1_format.csv', 'w')
sizefd = open('Questionnaire_Study1_size.csv', 'w')
ratiofd = open('Questionnaire_Study1_ratio.csv', 'w')

metric = ['Speed', 'Accuracy', 'Fatigue', 'Preference']
outfd.write('User')
sizefd.write('User')
ratiofd.write('User')
for m in metric:
    outfd.write(',' + m + '(1:1),' + m + '(1:3)')
    ratiofd.write(',' + m + '(1:1),' + m + '(1:3)')
    outfd.write(',' + m + '(1.25x),' + m + '(1.0x),' + m + '(0.75x)')
    sizefd.write(',' + m + '(1.25x),' + m + '(1.0x),' + m + '(0.75x)')
outfd.write('\n')
sizefd.write('\n')
ratiofd.write('\n')

lines = fd.readlines()
cnt = [([([0] * 5) for i in range(6)]) for i in range(4)]
for line in lines[1:]:
    data = line.decode('utf-8').split(',')
    outfd.write(data[0])
    p = 7
    f = 0
    score = [0] * 6
    for m in metric:
        for i in range(6):
            score[i] = float(data[p+i])
        outfd.write(',' + str(score[0] + score[1] + score[2]))
        outfd.write(',' + str(score[3] + score[4] + score[5]))
        outfd.write(',' + str(score[0] + score[3]))
        outfd.write(',' + str(score[1] + score[4]))
        outfd.write(',' + str(score[2] + score[5]))

        for i in range(6):
            s = int(score[i])
            if s < score[i]:
                s += 1

            if i < 3:
                cnt[f][i + 3][s - 1] += 1
            else:
                cnt[f][i - 3][s - 1] += 1
        f += 1
        p += 6

    outfd.write('\n')

    for i in range(3):
        ratiofd.write(data[0])
        p = 7
        for m in metric:
            ratiofd.write(',' + str(score[i]) + ',' + str(score[i + 3]))
            p += 6
        ratiofd.write('\n')
    for i in range(2):
        sizefd.write(data[0])
        p = 7
        for m in metric:
            sizefd.write(',' + str(score[i*3]) + ',' + str(score[i*3+1]) + ',' + str(score[i*3+2]))
            p += 6
        sizefd.write('\n')

chartfd = open('Questionnaire_Study1_chart.csv', 'w')
chartfd.write('type,1,2,3,4,5\n')
layouts = ['1:3&1.25x', '1:3&1.0x', '1:3&0.75x', '1:1&1.25x', '1:1&1.0x', '1:1&0.75x']
for i in range(4):
    for j in range(6):
        chartfd.write(metric[i] + "(" + layouts[j] + ")")
        for k in range(5):
            chartfd.write(',' + str(cnt[i][j][k]))
        chartfd.write('\n')


