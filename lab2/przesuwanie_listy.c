
			int pos = ftell(listOfPids); i=0;
			char movedByte = getc(listOfPids);
			while (movedByte != EOF && movedByte != '\0'){ //moving list to cover the gap
				fseek(listOfPids,pos + i + 1,SEEK_SET);
				movedByte = getc(listOfPids);
				if(movedByte == EOF){
					printf("---------->");
					break;
				}
				printf("pos end:%d ", pos + i + 1);
				fseek(listOfPids,p->posInFile + i,SEEK_SET);
				printf("pos start: %d %c\n", p->posInFile + i, movedByte);
				fwrite(&movedByte,1,1,listOfPids);
				i++;
			}
			movedByte = '\0';
			fseek(listOfPids,pos,SEEK_SET);
			fwrite(&movedByte, 1,1,listOfPids);
			fseek(listOfPids,1,SEEK_CUR);
			movedByte = EOF;
			fwrite(&movedByte, 1,1,listOfPids);