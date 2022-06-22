from os import fdopen

rawData = []
gaussValues = []

print("iniciando")
with open("./Data/data.txt", "r") as file:
    i = 0
    for linea in file.readlines():
        if i == 0:
            header = linea.rsplit(",")
            header.pop()
            print(len(header))
            print("header")
            print(header)
            for element in header:
                rawData.append(float(element))
        else:
            if linea.find("Gauss average") != -1:
                gline = linea.rsplit(": ")
                gaussValues.append(float(gline[1]))
            elif linea.find("81 A3 64 42 41") != -1:
                z = 3
                #print("msgpackdata")
            else:
                rawData.append(float(linea))
        i += 1

print("rawdata")
print(rawData)
print("gauss")
print(gaussValues)

with open("./Data/rawdata.txt","w") as file:
    i = 1
    for element in rawData:
        text = "{}, {}\n".format(i, element)
        file.write(text)
        i += 1

with open("./Data/gaussdata.txt", "w") as file:
    i = 95
    for element in gaussValues:
            text = "{}, {}\n".format(i, element)
            file.write(text)
            i += 1
print("finalizado")