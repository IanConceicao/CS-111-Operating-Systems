#NAME: Ian Conceicao
#EMAIL: IanCon234@gmail.com
#ID: 505153981
import sys
import math

problem = False
lines = ""

def printBlock(problem,blockNum,blocks):
    for reference in blocks[blockNum]:
        sys.stdout.write("{} {}BLOCK {} IN INODE {} AT OFFSET {}\n".format(problem,reference["Type"],blockNum,reference["Inode"],reference["Offset"]))

def block_analysis(inputFile):
    global problem
    global lines
    ###########################################
    # PART 1: Get Block Data into a JSon Structure
    #############################################

     #JSON Data structure:
                #  {BlockNumber : 
                #                 [{"Type" : ""/"Indirect"/"DOUBLE INDIRECT"/"TRIPLE INDIRECT",
                #                 "Offset": Offset,
                #                 "InodeNum":InodeNum,
                #                 "TimesRefd": # Of Times Block is referenced (should only be 1 if its legal)
                #                 }]
                #     ...
                #  }
    
    lines = inputFile.readlines()
    blocks = {}
    freeList = []
    for line in lines:
        fields = line.split(',')
        classification = fields[0]

        if classification == "SUPERBLOCK":
            BLOCK_SIZE = int(fields[3])
            INODE_SIZE = int(fields[4])
        
        elif classification == "GROUP":
            NUM_BLOCKS = int(fields[2])
            NUM_INODES = int(fields[3])
            iNodeLocal = int(fields[8])

    
        elif classification == "BFREE":
            blockNum = int(fields[1])
            freeList.append(blockNum)
        
        elif classification == "INODE":
            iNodeNum = int(fields[1])

            for i in range(12,27):
                if i >= len(fields):
                    break
                blockNum = int(fields[i])
                if blockNum == 0:
                    continue 

                if not blockNum in blocks.keys():
                    blocks[blockNum] = []

                adjustedI = i - 12

                if adjustedI < 12:
                    typeOfDirection = ""
                    offset = adjustedI
                elif adjustedI == 12:
                    typeOfDirection = "INDIRECT " #Extra space is for formatting later
                    offset = adjustedI
                elif adjustedI == 13:
                    typeOfDirection = "DOUBLE INDIRECT "
                    offset = 12 + 256
                elif adjustedI == 14:
                    typeOfDirection = "TRIPLE INDIRECT "
                    offset = 12 + 256 + 65536
                
                info = {}
                info["Offset"] = offset
                info["Type"]  = typeOfDirection
                info["Inode"] = iNodeNum

                blocks[blockNum].append(info)

        
        elif classification == "INDIRECT":
            
            blockNum = int(fields[5])
           
            if not blockNum in blocks.keys():
                blocks[blockNum] = []
           
            offset = int(fields[3])
            iNodeNum = int(fields[1])
            info = {}
            info["Offset"] = offset
            info["Type"]  = ""
            info["Inode"] = iNodeNum

            blocks[blockNum].append(info)


    FIRST_VALID_BLOCK = iNodeLocal + INODE_SIZE * NUM_INODES / BLOCK_SIZE
    
    ################################
    # PART 2: Analyze the Structure
    ################################

    #Do INVALID/RESERVED stuff first, and remove it if if its not legal
    for num in blocks.keys():
        if num < 0 or num >= NUM_BLOCKS:
            printBlock("INVALID",num,blocks)
            problem = True
            blocks.pop(num)
        if num > 0 and num < FIRST_VALID_BLOCK:
            printBlock("RESERVED",num,blocks)
            problem = True
            blocks.pop(num)

    #Find unreferenced blocks
    for i in range(FIRST_VALID_BLOCK, NUM_BLOCKS): #NUM_BLOCKS is total number of blocks, end case
        if not (i in blocks.keys() or i in freeList):
            sys.stdout.write("UNREFERENCED BLOCK {}\n".format(i))
            problem = True

    #Allocated Block but on free list
    for num in blocks.keys():
        if num in freeList:
            sys.stdout.write("ALLOCATED BLOCK {} ON FREELIST\n".format(num))
            problem = True

    #Block is Referenced by multiple files
    for num in blocks.keys():
        if len(blocks[num]) > 1:
            printBlock("DUPLICATE",num,blocks)
            problem = True
    
def i_node_and_dir_analysis():
    global problem
    NUM_INODES = 0
    #All ALLOCATED iNodes
    INodes = {} 
                # { INodeNumber :
                #         "LinkCount" : the link count given by the csv
                #          "Links" : the number of links found
                # }

    #All iNodes in free list
    freeINodes = []
    parentInodes = {}

    for line in lines:
        fields = line.split(',')
        classification = fields[0]

        if classification == "SUPERBLOCK":
            FIRST_INODE = int(fields[7])
        
        elif classification == "GROUP":
            NUM_INODES = int(fields[3])

        elif classification == "IFREE":
            INodeNum = int(fields[1])
            freeINodes.append(INodeNum)
        
        elif classification == "INODE":
            INodeNum = int(fields[1])
            INodes[INodeNum] = {}
            INodes[INodeNum]["LinkCount"] = int(fields[6])
            INodes[INodeNum]["Links"] = 0
    


    #Go through dirs later
    for line in lines:
        fields = line.split(',')
        classification = fields[0]

        if classification == "DIRENT":
            INodeEntry = int(fields[3])
            if INodeEntry in INodes.keys():
                INodes[INodeEntry]["Links"] = INodes[INodeEntry]["Links"] + 1 #Update link counts
            
            if not INodeEntry in parentInodes:
                parentInodes[INodeEntry] = int(fields[1])
                #Entry Node             Parent Node

    ##############
    #Dirs Analysis
    ##############
    for line in lines:

        fields = line.split(',')
        classification = fields[0]

        if classification == "DIRENT":      

            INodeEntry = int(fields[3])
            INodeDir = int(fields[1])
            entryName = fields[6]
            entryName = entryName.rstrip("\n")
           
          

            if INodeEntry not in INodes.keys() and INodeEntry in range(FIRST_INODE, NUM_INODES):
                sys.stdout.write("DIRECTORY INODE {} NAME {} UNALLOCATED INODE {}\n".format(INodeDir,entryName,INodeEntry))
            if INodeEntry < 1 or INodeEntry >= NUM_INODES:
                sys.stdout.write("DIRECTORY INODE {} NAME {} INVALID INODE {}\n".format(INodeDir,entryName,INodeEntry))
            if entryName == "'.'" and INodeEntry != INodeDir:
                sys.stdout.write("DIRECTORY INODE {} NAME '.' LINK TO INODE {} SHOULD BE {}\n".format(INodeDir, INodeEntry, INodeDir))
            if entryName == "'..'" and INodeEntry != parentInodes[INodeDir]:
                sys.stdout.write("DIRECTORY INODE {} NAME '..' LINK TO INODE {} SHOULD BE {}\n".format(INodeDir, INodeEntry, parentInodes[INodeDir]))


    #Go through dirs here

    #########
    # INodes Analysis
    #########

    #Make sure all allocated are not on free list
    for i in INodes.keys():
        if i in freeINodes:
            sys.stdout.write("ALLOCATED INODE {} ON FREELIST\n".format(i))
            problem = True
    
    #Make sure all unallocated INodes are on free list
    for i in range(FIRST_INODE, NUM_INODES):
        if (not (i in freeINodes)) and (not (i in INodes.keys())):
            sys.stdout.write("UNALLOCATED INODE {} NOT ON FREELIST\n".format(i))
            problem = True

    #Make sure all links match link counts
    for i in INodes.keys():
        if INodes[i]["Links"] != INodes[i]["LinkCount"]:
            sys.stdout.write("INODE {} HAS {} LINKS BUT LINKCOUNT IS {}\n".format(i,INodes[i]["Links"],INodes[i]["LinkCount"]))
            problem = True





def main():
    if len(sys.argv) != 2:
        sys.stderr.write("Usage: lab3b <fileName.csv>\n")
        exit(1)
    
    try:
        inputFile = open(sys.argv[1],'r')
    except IOError:
        sys.stderr.write("Error: Could not open file\n")
        exit(2)
    
    block_analysis(inputFile)
    i_node_and_dir_analysis()


    if problem == True:
        exit(2)

    exit(0)


if __name__ == "__main__":
    main()