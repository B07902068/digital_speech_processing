import sys



if __name__ == '__main__':
	inFile = open(sys.argv[1], "r", encoding = 'big5-hkscs')
	outFile = open(sys.argv[2], "w", encoding = 'big5-hkscs')

	ZhuYin_dict = dict()
	content = inFile.readline()
	while content != "":
		current = 2
		if content[current] in ZhuYin_dict:
			ZhuYin_dict[content[current]].append(content[0])
		else:
			ZhuYin_dict[content[current]] = list() 
			ZhuYin_dict[content[current]].append(content[0])
		current = content.find('/', current)
		while current > 0:
			current += 1
			if content[current] in ZhuYin_dict:
				if content[0] not in ZhuYin_dict[content[current]]: 
					ZhuYin_dict[content[current]].append(content[0])
			else:
				ZhuYin_dict[content[current]] = list() 
				ZhuYin_dict[content[current]].append(content[0])
			current = content.find('/', current)
		content = inFile.readline()
	
	for z, m in sorted(ZhuYin_dict.items()):
		outFile.write(z + " " + " ".join(m) + '\n')
		for i in m:
			outFile.write(i + " " + i + '\n')
	inFile.close()
	outFile.close()
	
