/******************************************************************************
 *
 * Project:  MapServer
 * Purpose:  Commandline utility to ... sort shapes?  Not sure what its for.
 * Author:   Steve Lime and the MapServer team.
 *
 ******************************************************************************
 * Copyright (c) 1996-2004 Regents of the University of Minnesota.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in 
 * all copies of this Software or works derived from this Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 ******************************************************************************
 *
 * $Log$
 * Revision 1.3  2004/10/21 04:30:55  frank
 * Added standardized headers.  Added MS_CVSID().
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "map.h"

MS_CVSID("$Id$")

typedef struct {
  double number;
  char string[255];
  int index;
} sortStruct;

static int compare_string_descending(i,j)
sortStruct *i, *j; 
{
  return(strcmp(j->string, i->string));
}

static int compare_string_ascending(i,j) 
sortStruct *i, *j;
{
  return(strcmp(i->string, j->string));
}

static int compare_number_descending(i,j)
sortStruct *i, *j; 
{
  if(i->number > j->number)
    return(-1);  
  if(i->number < j->number)  
    return(1);
  return(0);
}

static int compare_number_ascending(i,j) 
sortStruct *i, *j;
{
  if(i->number > j->number)
    return(1);  
  if(i->number < j->number)  
    return(-1);
  return(0);
}

int main(argc,argv)
int argc;
char *argv[];
{
  SHPHandle    inSHP,outSHP; /* ---- Shapefile file pointers ---- */
  DBFHandle    inDBF,outDBF; /* ---- DBF file pointers ---- */  
  sortStruct   *array;
  shapeObj     shape;
  int          shpType, nShapes;
  int          fieldNumber=-1; /* ---- Field number of item to be sorted on ---- */
  DBFFieldType dbfField;
  char         fName[20];
  int          fWidth,fnDecimals; 
  char         buffer[1024];
  int i,j;
  int num_fields, num_records;
  
  if(argc > 1 && strcmp(argv[1], "-v") == 0) {
    printf("%s\n", msGetVersion());
    exit(0);
  }

  /* ------------------------------------------------------------------------------- */
  /*       Check the number of arguments, return syntax if not correct               */
  /* ------------------------------------------------------------------------------- */
  if( argc != 5 ) {
      fprintf(stderr,"Syntax: sortshp [infile] [outfile] [item] [ascending|descending]\n" );
      exit(0);
  }

  /* ------------------------------------------------------------------------------- */
  /*       Open the shapefile                                                        */
  /* ------------------------------------------------------------------------------- */
  inSHP = msSHPOpen(argv[1], "rb" );
  if( !inSHP ) {
    fprintf(stderr,"Unable to open %s shapefile.\n",argv[1]);
    exit(0);
  }
  msSHPGetInfo(inSHP, &nShapes, &shpType);

  /* ------------------------------------------------------------------------------- */
  /*       Open the dbf file                                                         */
  /* ------------------------------------------------------------------------------- */
  sprintf(buffer,"%s.dbf",argv[1]);
  inDBF = msDBFOpen(buffer,"rb");
  if( inDBF == NULL ) {
    fprintf(stderr,"Unable to open %s XBASE file.\n",buffer);
    exit(0);
  }

  num_fields = msDBFGetFieldCount(inDBF);
  num_records = msDBFGetRecordCount(inDBF);

  for(i=0;i<num_fields;i++) {
    msDBFGetFieldInfo(inDBF,i,fName,NULL,NULL);
    if(strncasecmp(argv[3],fName,strlen(argv[3])) == 0) { /* ---- Found it ---- */
      fieldNumber = i;
      break;
    }
  }

  if(fieldNumber < 0) {
    fprintf(stderr,"Item %s doesn't exist in %s\n",argv[3],buffer);
    exit(0);
  }  

  array = (sortStruct *)malloc(sizeof(sortStruct)*num_records); /* ---- Allocate the array ---- */
  if(!array) {
    fprintf(stderr, "Unable to allocate sort array.\n");
    exit(0);
  }
  
  /* ------------------------------------------------------------------------------- */
  /*       Load the array to be sorted                                               */
  /* ------------------------------------------------------------------------------- */
  dbfField = msDBFGetFieldInfo(inDBF,fieldNumber,NULL,NULL,NULL);
  switch (dbfField) {
  case FTString:
    for(i=0;i<num_records;i++) {
      strcpy(array[i].string, msDBFReadStringAttribute( inDBF, i, fieldNumber));
      array[i].index = i;
    }

    if(*argv[4] == 'd')
      qsort(array, num_records, sizeof(sortStruct), compare_string_descending);
    else
      qsort(array, num_records, sizeof(sortStruct), compare_string_ascending);
    break;
  case FTInteger:
  case FTDouble:
    for(i=0;i<num_records;i++) {
      array[i].number = msDBFReadDoubleAttribute( inDBF, i, fieldNumber);
      array[i].index = i;
    }

    if(*argv[4] == 'd')
      qsort(array, num_records, sizeof(sortStruct), compare_number_descending);
    else
      qsort(array, num_records, sizeof(sortStruct), compare_number_ascending);

    break;
  default:
      fprintf(stderr,"Data type for item %s not supported.\n",argv[3]);
      exit(0);
  } 
  
  /* ------------------------------------------------------------------------------- */
  /*       Setup the output .shp/.shx and .dbf files                                 */
  /* ------------------------------------------------------------------------------- */
  outSHP = msSHPCreate(argv[2],shpType);
  sprintf(buffer,"%s.dbf",argv[2]);
  outDBF = msDBFCreate(buffer);

  for(i=0;i<num_fields;i++) {
    dbfField = msDBFGetFieldInfo(inDBF,i,fName,&fWidth,&fnDecimals); /* ---- Get field info from in file ---- */
    msDBFAddField(outDBF,fName,dbfField,fWidth,fnDecimals);
  }

  /* ------------------------------------------------------------------------------- */
  /*       Write the sorted .shp/.shx and .dbf files                                 */
  /* ------------------------------------------------------------------------------- */
  for(i=0;i<num_records;i++) { /* ---- For each shape/record ---- */

    for(j=0;j<num_fields;j++) { /* ---- For each .dbf field ---- */

      dbfField = msDBFGetFieldInfo(inDBF,j,fName,&fWidth,&fnDecimals); 

      switch (dbfField) {
      case FTInteger:
	msDBFWriteIntegerAttribute(outDBF, i, j, msDBFReadIntegerAttribute( inDBF, array[i].index, j));
        break;
      case FTDouble:
	msDBFWriteDoubleAttribute(outDBF, i, j, msDBFReadDoubleAttribute( inDBF, array[i].index, j));
	break;
      case FTString:
	msDBFWriteStringAttribute(outDBF, i, j, msDBFReadStringAttribute( inDBF, array[i].index, j));
	break;
      default:
	fprintf(stderr,"Unsupported data type for field: %s, exiting.\n",fName);
        exit(0);
      }
    }
    
    msSHPReadShape( inSHP, array[i].index, &shape );
    msSHPWriteShape( outSHP, &shape );
  }

  free(array);

  msSHPClose(inSHP);
  msDBFClose(inDBF);
  msSHPClose(outSHP);
  msDBFClose(outDBF);

  return(0);
}
