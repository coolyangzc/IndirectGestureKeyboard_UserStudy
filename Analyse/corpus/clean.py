fd = file('ANC-written-count.txt')
outfd = open('corpus-written.txt', 'w')
lines = fd.readlines()
for line in lines:
    line_data = line.split('\t');
    word = line_data[0]
    if word.isalpha():
        outfd.write(word + '    ' + line_data[-1]);