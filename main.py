


def main():
    numbers = list()
    with open("origin.txt") as hFile:
        for line in hFile.readlines():
            line=line.strip()
            if line :
                try:
                    numbers.append(int(line))
                except ValueError as v:
                    pass
            pass
        pass
    # print(len(numbers))

    fineNumber = list()
    with open("test.txt") as hFile:
        for line in hFile.readlines():
            line=line.strip()
            line=line.strip()
            if not line:
                continue
            if not line.startswith("{"):
                continue
            pos = line.index("}",1)
            line = line[1:pos]
            try:
                fineNumber.append(int(line))
            except ValueError as v:
                pass
    # print(len(fineNumber))
    for n in fineNumber:
        if n not in numbers:
            print (n)
            return


if __name__ == "__main__":
    main()
    print("finished")
