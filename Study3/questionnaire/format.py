fd = file('Questionnaire_Study1.csv')
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

    for i in range(3):
        ratiofd.write(data[0])
        p = 7
        for m in metric:
            score = map(float, data[p:p + 6])
            ratiofd.write(',' + str(score[i]) + ',' + str(score[i + 3]))
            p += 6
        ratiofd.write('\n')
    for i in range(2):
        sizefd.write(data[0])
        p = 7
        for m in metric:
            score = map(float, data[p:p + 6])
            sizefd.write(',' + str(score[i*3]) + ',' + str(score[i*3+1]) + ',' + str(score[i*3+2]))
            p += 6
        sizefd.write('\n')

