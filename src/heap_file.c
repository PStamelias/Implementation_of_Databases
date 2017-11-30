#include "heap_file.h"
#include "bf.h"
#include <stdio.h>
#include <string.h>
HP_ErrorCode HP_Init() {
  return HP_OK;
}

HP_ErrorCode HP_CreateIndex(const char *filename) {

  BF_ErrorCode err;
  int file_desk=-1;
  BF_Block * block1,*block2 ;
  BF_Block_Init(&block1);
  err=BF_CreateFile(filename);
  if(err!=BF_OK)
  {
	BF_PrintError(err);
	BF_Block_Destroy(&block1);
	return HP_ERROR;
  }
  err=BF_OpenFile(filename,&file_desk);
  if(err!=BF_OK)
  {
	BF_PrintError(err);
	BF_Block_Destroy(&block1);
	return HP_ERROR;
  }
  err=BF_AllocateBlock(file_desk,block1);
  if(err!=BF_OK)
  {
	BF_PrintError(err);
	BF_Block_Destroy(&block1);
	return HP_ERROR;
  }
  char  *p=BF_Block_GetData(block1);
  *(char *)p='H';
  BF_Block_SetDirty(block1);
  err=BF_UnpinBlock(block1);
  if(err!=BF_OK)
  {
 	BF_PrintError(err);
 	BF_Block_Destroy(&block1);
	return HP_ERROR;
  }
  BF_Block_Init(&block2);
  err=BF_AllocateBlock(file_desk,block2);
  char    *poin=BF_Block_GetData(block2);
  int zero=0;
  memcpy(poin,&zero,sizeof(int));
  BF_Block_SetDirty(block2);
  err=BF_UnpinBlock(block2);
  if(err!=BF_OK)
  {
	BF_PrintError(err);
	BF_Block_Destroy(&block1);
    	BF_Block_Destroy(&block2);
	return HP_ERROR;
  }
  err=BF_CloseFile(file_desk);
  if(err!=BF_OK)
  {
	BF_PrintError(err);
	BF_Block_Destroy(&block1);
    	BF_Block_Destroy(&block2);
	return HP_ERROR;
  }
  BF_Block_Destroy(&block1);
  BF_Block_Destroy(&block2);
  return HP_OK;
}

HP_ErrorCode HP_OpenFile(const char *fileName, int *fileDesc){
  BF_ErrorCode err;
  BF_Block* block3;
  err=BF_OpenFile(fileName,fileDesc);
  if(err!=BF_OK)
  {
	BF_PrintError(err);
	return HP_ERROR;
  }
  BF_Block_Init(&block3);
  err=BF_GetBlock(*fileDesc,0,block3);
  if(err!=BF_OK)
  {
	BF_PrintError(err);
	BF_Block_Destroy(&block3);
	return HP_ERROR;
  }
  char* p=BF_Block_GetData(block3);
  if((*(int*)p)=='H')
  {
	err=BF_UnpinBlock(block3);
	BF_Block_Destroy(&block3);
	if(err!=BF_OK)
	{
		BF_PrintError(err);
		return HP_ERROR;
	}
  }
  else
  {
	BF_Block_Destroy(&block3);
	return HP_ERROR;
  }
  return HP_OK;
}

HP_ErrorCode HP_CloseFile(int fileDesc){
  BF_ErrorCode err;
  err=BF_CloseFile(fileDesc);
  if(err!=BF_OK)
  {
	BF_PrintError(err);
	return HP_ERROR;
  }
  return HP_OK;
}

HP_ErrorCode HP_InsertEntry(int fileDesc, Record record) {
  BF_ErrorCode err;
  BF_Block* block;
  int block_nums=0;
  err=BF_GetBlockCounter(fileDesc,&block_nums);
  if(err!=BF_OK)
  {
	BF_PrintError(err);
	return HP_ERROR;
  }
  BF_Block_Init(&block);
  err=BF_GetBlock(fileDesc,block_nums-1,block);
  if(err!=BF_OK)
  {
	BF_PrintError(err);
	BF_Block_Destroy(&block);
	return HP_ERROR;
  }
  char *p=BF_Block_GetData(block);
  int curr_records=*(int*)p;
  int  max_record=(BF_BLOCK_SIZE-sizeof(int))/sizeof(Record);
  if(curr_records<max_record)
  {
	memcpy(p+sizeof(int)+curr_records*sizeof(Record),&record,sizeof(Record));
	*(int*)p=curr_records+1;
	BF_Block_SetDirty(block);
	err=BF_UnpinBlock(block);
	if(err!=BF_OK)
	{
		BF_Block_Destroy(&block);
		BF_PrintError(err);
		return HP_ERROR;
	}
  }
  else
  {
	err=BF_UnpinBlock(block);
	if(err!=BF_OK)
	{
		BF_PrintError(err);
		BF_Block_Destroy(&block);
		return HP_ERROR;
	}
	BF_Block * block2;
	BF_Block_Init(&block2);
	err=BF_AllocateBlock(fileDesc,block2);
	char *p=BF_Block_GetData(block2);
	*(int*)p=1;
	memcpy(p+sizeof(int),&record,sizeof(Record));
	BF_Block_SetDirty(block2);
	err=BF_UnpinBlock(block2);
	if(err!=BF_OK)
	{
		BF_PrintError(err);
		BF_Block_Destroy(&block);
		BF_Block_Destroy(&block2);
		return HP_ERROR;
	}
  	BF_Block_Destroy(&block2);
  }
  BF_Block_Destroy(&block);
  return HP_OK;
}

HP_ErrorCode HP_PrintAllEntries(int fileDesc) {
  Record temp;
  char *p;
  int curr_records=0;
  BF_Block* block;
  BF_Block_Init(&block);
  BF_ErrorCode err;
  int block_num=0,i,j;
  err=BF_GetBlockCounter(fileDesc,&block_num);
  for(i=1;i<block_num;i++)
  {

	err=BF_GetBlock(fileDesc,i,block);
	if(err!=BF_OK)
	{
		BF_PrintError(err);
		BF_Block_Destroy(&block);
		return HP_ERROR;
	}
	p=BF_Block_GetData(block);
	curr_records=*(int*)p;
	for(j=0;j<curr_records;j++)
	{
		memcpy(&temp,p+sizeof(int)+j*sizeof(Record),sizeof(Record));
		printf("\n%d %s %s %s",temp.id,temp.name,temp.surname,temp.city);
	}
	err=BF_UnpinBlock(block);
	if(err!=BF_OK)
	{
		BF_PrintError(err);
		BF_Block_Destroy(&block);
		return HP_ERROR;
	}
  }
  printf("\n");
  BF_Block_Destroy(&block);
  return HP_OK;
}

HP_ErrorCode HP_GetEntry(int fileDesc, int rowId, Record *record) {
  int blocks_num=0;
  BF_ErrorCode err;
  err=BF_GetBlockCounter(fileDesc,&blocks_num);
  BF_Block* block;
  if(err!=BF_OK)
  {
	BF_ErrorCode(err);
	return HP_ERROR;
  }
  int max_records=(BF_BLOCK_SIZE-sizeof(int))/sizeof(Record);
  int block_no=(rowId-1)/max_records;
  block_no++;
  if(block_no>blocks_num)
	return HP_ERROR;
  int thesi=(rowId-1)%max_records;
  BF_Block_Init(&block);
  err=BF_GetBlock(fileDesc,block_no,block);
  if(err!=BF_OK)
  {
	BF_Block_Destroy(&block);
	BF_ErrorCode(err);
	return HP_ERROR;
  }
  char *p=BF_Block_GetData(block);
  int curr_records=*(int*)p;
  if(thesi>=curr_records)
  {
	BF_Block_Destroy(&block);
	return HP_ERROR;
  }
  else
	memcpy(record,p+sizeof(int)+thesi*sizeof(Record),sizeof(Record));
  err=BF_UnpinBlock(block);
  if(err!=BF_OK)
  {
	BF_PrintError(err);
	BF_Block_Destroy(&block);
	return HP_ERROR;
  }
  BF_Block_Destroy(&block);
  return HP_OK;
}
