//NAME: Ian Conceicao
//EMAIL: IanCon234@gmail.com
//ID: 505153981

#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include<signal.h> 
#include <time.h>
#include "ext2_fs.h"
#include <math.h> 
#include <unistd.h>
#include <stdint.h>


unsigned int BLOCK_SIZE;

const int SUPERBLOCK_OFFSET = 1024;

int imageFD;

struct ext2_super_block superBlock;
struct ext2_group_desc groupDesc;

int numOfGroups;

unsigned long blockOffset(unsigned int block){
    return (block - 1) * BLOCK_SIZE + SUPERBLOCK_OFFSET;
}

unsigned long groupOffset(int groupNum){
    return SUPERBLOCK_OFFSET + BLOCK_SIZE + groupNum * sizeof(struct ext2_group_desc);
}

void convertTimes(time_t cTime, time_t mTime, time_t aTime, char* creationTime, char* modificationTime, char* accessTime){
    //Ctime
    time_t temp = cTime;
    struct tm* timeStructure = gmtime(&temp);
    strftime(creationTime,40,"%m/%d/%y %H:%M:%S",timeStructure);
    
    //Mtime
    temp = mTime;
    timeStructure = gmtime(&temp);
    strftime(modificationTime,40,"%m/%d/%y %H:%M:%S",timeStructure);

    //aTime
    temp = aTime;
    timeStructure = gmtime(&temp);
    strftime(accessTime,40,"%m/%d/%y %H:%M:%S",timeStructure);
}


void directoryEntries(unsigned int blockNumber, unsigned int inode) {
	struct ext2_dir_entry dirEntry;
	unsigned int numberOfBytes = 0;

	for(;numberOfBytes < BLOCK_SIZE; numberOfBytes += dirEntry.rec_len){
		pread(imageFD, &dirEntry, sizeof(dirEntry), blockOffset(inode) + numberOfBytes);
		if (dirEntry.inode) {
			fprintf(stdout, 
                "DIRENT,%d,%d,%d,%d,%d,'%s'\n",
				blockNumber, 
				numberOfBytes, 
				dirEntry.inode, 
				dirEntry.rec_len, 
				dirEntry.name_len, 
				dirEntry.name);
		}
	}
}


//1 means allocated, 0 means free
void freeBlockEntries(int groupNum, unsigned int fistBlockBitmap){
    
    unsigned long offset = blockOffset(fistBlockBitmap);
    char* contents = (char* ) malloc(BLOCK_SIZE);
    pread(imageFD, contents, BLOCK_SIZE, offset);

    unsigned int blockNum = superBlock.s_first_data_block + (groupNum * superBlock.s_blocks_per_group);
    unsigned int i=0;
    unsigned int j;
    int allocated;
    for(;i< BLOCK_SIZE; i++){
        j=0;
        char byte = contents[i];
        for(;j < 8; j++){

            allocated = 1 & (byte >> j);
            if(!allocated)
                fprintf(stdout,"BFREE,%d\n",blockNum);
            
            blockNum ++;
        }
    }
    free(contents);
}

void INodeSummary(unsigned int INodeNum, unsigned int INodeTable, unsigned int index){
    struct ext2_inode inodeStruct;

    pread(imageFD, &inodeStruct, sizeof(inodeStruct), blockOffset(INodeTable) + index * sizeof(inodeStruct));

   if(inodeStruct.i_mode != 0 && inodeStruct.i_links_count != 0){

    char creationTime[30], modificationTime[30], accessTime[30];
    
    convertTimes(inodeStruct.i_ctime,inodeStruct.i_mtime,inodeStruct.i_atime,creationTime,modificationTime,accessTime);
    
    char fileType = '0';
    u_int16_t fileMode = inodeStruct.i_mode >> 12;
    fileMode = fileMode << 12;
    switch(fileMode){
        case 0x8000:
            fileType = 'f';
            break;
        case 0xA000:
            fileType = 's';
            break;
        case 0x4000:
            fileType = 'd';
            break;
        default:
             fileType = '?';
             break;
    }
   

    fprintf(stdout, 
        "INODE,%d,%c,%o,%d,%d,%d,%s,%s,%s,%d,%d",
        INodeNum,
		fileType, 
		inodeStruct.i_mode & 0xFFF,
		inodeStruct.i_uid, 
		inodeStruct.i_gid, 
		inodeStruct.i_links_count,
        creationTime,
        modificationTime,
        accessTime,
        inodeStruct.i_size,
		inodeStruct.i_blocks);

    unsigned int i = 0;
    unsigned int j = 0;
    unsigned int k = 0;
    
    //Print out 15 block points
    if(fileType != 's' || (inodeStruct.i_size > 60)){ 
        for(;i < 15; i++)
            fprintf(stdout, ",%d", inodeStruct.i_block[i]);
    }
    fprintf(stdout,"\n");

    //Directory entries if directory
    if(fileType == 'd'){
        for(i=0;i < 12;i++){
            if(inodeStruct.i_block[i])
                directoryEntries(INodeNum, inodeStruct.i_block[i]);
        }
    }

    
    //Indirect block references
    if(fileType == 'd' || fileType == 'f'){
           
            //Single indirect
            if(inodeStruct.i_block[12] > 0){
                int indirection[BLOCK_SIZE];

                pread(imageFD,indirection,BLOCK_SIZE,inodeStruct.i_block[12] * BLOCK_SIZE);
                
                for(i=0;i<BLOCK_SIZE / sizeof(int);i++){
                    if(indirection[i]){
                        fprintf(stdout, 
                            "INDIRECT,%d,%d,%d,%d,%d\n",
                            INodeNum,
                            1,
                            i+12,
                            inodeStruct.i_block[12],
                            indirection[i] );

                        if(fileType == 'd')
                            directoryEntries(INodeNum,indirection[i]); 

                    }
                }
            }

            //Double indirection
            if(inodeStruct.i_block[13] > 0){
                int level2 [BLOCK_SIZE];
                int level1 [BLOCK_SIZE];

                pread(imageFD,level2, BLOCK_SIZE, inodeStruct.i_block[13] * BLOCK_SIZE);
                for(i=0;i<BLOCK_SIZE/sizeof(int);i++){
                    if(level2[i]){
                        fprintf(stdout, 
                            "INDIRECT,%d,%d,%d,%d,%d\n",
                            INodeNum,
                            2,
                            i+12+256,
                            inodeStruct.i_block[13],
                            level2[i]);
                    
                    
                        pread(imageFD,level1, BLOCK_SIZE, blockOffset(level2[i]));

                        for(j=0;j<BLOCK_SIZE / sizeof(int);j++)
                        if(level1[j]){
                            
                            fprintf(stdout, 
                                "INDIRECT,%d,%d,%d,%d,%d\n",
                                INodeNum,
                                1,
                                j+12+256,
                                level2[i],
                                level1[j]);
                            
                            if(fileType == 'd')
                                directoryEntries(INodeNum,level1[j]);
                        }
                    }
                }        
            }

            //Tripler indirection
            if(inodeStruct.i_block[14] > 0){
                int level3 [BLOCK_SIZE];
                int level2 [BLOCK_SIZE];
                int level1 [BLOCK_SIZE];

                pread(imageFD,level3,BLOCK_SIZE,blockOffset(inodeStruct.i_block[14]));

                for(i=0;i < BLOCK_SIZE/sizeof(int);i++){
                    if(level3[i]){
                        fprintf(stdout, 
                        "INDIRECT,%d,%d,%d,%d,%d\n",
                        INodeNum,
                        3,
                        i + 12 + 256 + 65536,
                        inodeStruct.i_block[14], 
                        level3[i]);

                        pread(imageFD,level2,BLOCK_SIZE,blockOffset(level3[i]));
                        
                        for(j=0;j<BLOCK_SIZE/sizeof(int);j++){
                            if(level2[j]){
                                fprintf(stdout, 
                                "INDIRECT,%d,%d,%d,%d,%d\n",
                                INodeNum,
                                2, 
                                65536 + 256 + 12 + j, 
                                level3[i],
                                level2[j]);	
                            
                                pread(imageFD,level1,BLOCK_SIZE,blockOffset(level2[j]));

                                
                                    for(k=0;k<BLOCK_SIZE/sizeof(int);k++){
                                        if(level1[k]){
                                        fprintf(stdout, 
                                        "INDIRECT,%d,%d,%d,%d,%d\n",
                                        INodeNum,
                                        1, 
                                        k + 12 + 256 + 65536, 
                                        level2[j],
                                        level1[k]);

                                        if(fileType == 'd')
                                            directoryEntries(INodeNum,level1[k]); 

                            }
                        }
                        }
                    }
                }
            }
    }
    }
   }
}


void freeInodeEntries(int groupNum, int firstInodeBitmap, int firstInodeTable){
    unsigned long offset = blockOffset(firstInodeBitmap);
    unsigned int numberOfInodesBytes = superBlock.s_inodes_per_group / 8;
   
    char contents[numberOfInodesBytes]; //May not work on old linux server
    unsigned int INodeNum = groupNum * superBlock.s_inodes_per_group + 1;
    unsigned int head = INodeNum;

    pread(imageFD, contents, numberOfInodesBytes, offset);
    unsigned int i=0;
    unsigned int j;
    int allocated;
    int index;
    char byte;
    for(;i< numberOfInodesBytes; i++){
        j=0;
        byte  = contents[i];
        for(;j < 8; j++){
            allocated = 1 & (byte >> j);
            index = INodeNum - head;
            if(!allocated)
                fprintf(stdout,"IFREE,%d\n",INodeNum);
            else
                INodeSummary(INodeNum, firstInodeTable, index);
            
            INodeNum ++;
        }
    }

   // free(contents);

}



void groupSummary(int groupNum){
    struct ext2_group_desc groupDesc;
    unsigned long offset = groupOffset(groupNum);

    unsigned int numOfBlocks = superBlock.s_blocks_per_group;
    unsigned int numOfInodes = superBlock.s_inodes_per_group;
    //If last group
    if(groupNum + 1 == numOfGroups){
        numOfBlocks = superBlock.s_blocks_count % superBlock.s_blocks_per_group; //leftovers
        numOfInodes = superBlock.s_inodes_count % superBlock.s_inodes_per_group;
    }
    if(numOfInodes == 0){
        numOfInodes = superBlock.s_inodes_per_group;
    }
    if(numOfBlocks == 0){
        numOfBlocks = superBlock.s_blocks_per_group;
    }

    pread(imageFD, &groupDesc, sizeof(groupDesc), offset);

    fprintf(stdout,
        "GROUP,%d,%d,%d,%d,%d,%d,%d,%d\n",
		groupNum, 
		numOfBlocks, 
		numOfInodes,
		groupDesc.bg_free_blocks_count,
		groupDesc.bg_free_inodes_count, 
		groupDesc.bg_block_bitmap, 
		groupDesc.bg_inode_bitmap, 
		groupDesc.bg_inode_table
    );

    freeBlockEntries(groupNum, groupDesc.bg_block_bitmap);
    freeInodeEntries(groupNum, groupDesc.bg_inode_bitmap, groupDesc.bg_inode_table);
}


void iterateGroups(){
    double numOfGroupsDbl = (double) superBlock.s_blocks_count / (double) superBlock.s_blocks_per_group;
    numOfGroups = ceil(numOfGroupsDbl); //Rounds up
    //fprintf(stderr,"Found %f groups\n",numOfGroupsDbl);
    //fprintf(stderr,"Found %d groups\n",numOfGroups);

    int i = 0;
    for(;i < numOfGroups; i++){
        groupSummary(i); //Will call child functions as well
        //printf(stderr,"Calling group summary\n");
    }
}


void superBlockSummary(){
    pread(imageFD, &superBlock, sizeof(superBlock), SUPERBLOCK_OFFSET);

    fprintf(stdout,
            "SUPERBLOCK,%d,%d,%d,%d,%d,%d,%d\n",
            superBlock.s_blocks_count,
            superBlock.s_inodes_count,
            BLOCK_SIZE,
            superBlock.s_inode_size,
            superBlock.s_blocks_per_group,
            superBlock.s_inodes_per_group,
            superBlock.s_first_ino
    );
}

int main(int argc, char** argv){

    if(argc != 2){
        fprintf(stderr,"Please enter a single directory name.\n");
        exit(1);
    }

    imageFD = open(argv[1], O_RDONLY);
    if(imageFD < 0){
        fprintf(stderr,"Error opening the directory given.\n");
        exit(1);
    }

    BLOCK_SIZE = EXT2_MIN_BLOCK_SIZE << superBlock.s_log_block_size;
    superBlockSummary();
    iterateGroups();

    exit(0);
}