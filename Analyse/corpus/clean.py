fd = file('ANC-written-count.txt')
outfd_clean = open('ANC-written-clean.txt', 'w')
outfd_nodup = open('ANC-written-noduplicate.txt', 'w')
dict = {}
lines = fd.readlines()
for line in lines:
    line_data = line.split('\t');
    word = line_data[0]
    if not word.isalpha():
        continue
    if dict.has_key(word):
        dict[word] += int(line_data[-1])
    else:
        dict[word] = int(line_data[-1])
    outfd_clean.write(word + ' ' + line_data[-1])
sorted_dict = sorted(dict.items(), key=lambda item:item[1], reverse=True)
for word in sorted_dict:
    outfd_nodup.write(word[0] + ' ' + str(word[1]) + '\n')
