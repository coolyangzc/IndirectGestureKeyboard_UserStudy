fd = file('Questionnaire_Study1.csv')
outfd = open('Questionnaire_Study1_format.csv', 'w')

outfd.write('User')
metric = ['Speed', 'Accuracy', 'Fatigue', 'Preference']
for m in metric:
    outfd.write(',' + m + '(1:1),' + m + '(1:3)')
    outfd.write(',' + m + '(1.25x),' + m + '(1.0x),' + m + '(0.75x)')
outfd.write('\n')

lines = fd.readlines()
for line in lines[1:]:
    data = line.split(',')
    outfd.write(data[0])
    p = 7
    for m in metric:
        score = map(float, data[p:p+6])
        outfd.write(',' + str(score[0] + score[1] + score[2]))
        outfd.write(',' + str(score[3] + score[4] + score[5]))
        outfd.write(',' + str(score[0] + score[3]))
        outfd.write(',' + str(score[1] + score[4]))
        outfd.write(',' + str(score[2] + score[5]))
        p += 6
    outfd.write('\n')

