/*Includes*/
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>


#define BUFFER_SIZE 1024

/*Structure to hold conf file data*/
typedef struct server_info_t{
  char server_ip1[20];
  char server_ip2[20];
  char server_ip3[20];
  char server_ip4[20];
  char server_port1[10];
  char server_port2[10];
  char server_port3[10];
  char server_port4[10];
  char username[2][20];
  char password[2][20];

}server_info;

server_info info;


/*Function to parse file*/
void parse_file(){
  char buffer[BUFFER_SIZE];
  char server1[5]="DFS1";
  char server2[5]="DFS2";
  char server3[5]="DFS3";
  char server4[5]="DFS4";
  char username1[11]="Username1:";
  char password1[11]="Password1:";
  char username2[11]="Username2:";
  char password2[11]="Password2:";

  int size=0;
  char* serv1;
  char* serv2;
  char* serv3;
  char* serv4;
  char* user1;
  char* pass1;
  char* user2;
  char* pass2;

  FILE* conf_file;
  conf_file=fopen("client.conf","r");

  fseek(conf_file,0,SEEK_END);
  size=ftell(conf_file);
  fseek(conf_file,0,SEEK_SET);

  fread(buffer,1,size,conf_file);

  fclose(conf_file);

  serv1=strstr(buffer,server1);
  serv1=serv1 + strlen(server1)+1;

  serv2=strstr(buffer,server2);
  serv2=serv2+ strlen(server2)+1;

  serv3=strstr(buffer,server3);
  serv3=serv3+ strlen(server3)+1;

  serv4=strstr(buffer,server4);
  serv4=serv4+ strlen(server4)+1;

  user1=strstr(buffer,username1);
  user1=user1 + strlen(username1)+1;

  pass1=strstr(buffer,password1);
  pass1=pass1 + strlen(password1)+1;


  user2=strstr(buffer,username2);
  user2=user2 + strlen(username2)+1;


  pass2=strstr(buffer,password2);
  pass2=pass2 + strlen(password2)+1;


  serv1=strtok(serv1,"\n");

  serv1=strtok(serv1,":");
  strcpy(info.server_ip1,serv1);

  serv1=strtok(NULL," ");
  strcpy(info.server_port1,serv1);


  serv2=strtok(serv2,"\n");

  serv2=strtok(serv2,":");
  strcpy(info.server_ip2,serv2);

  serv2=strtok(NULL," ");
  strcpy(info.server_port2,serv2);


  serv3=strtok(serv3,"\n");

  serv3=strtok(serv3,":");
  strcpy(info.server_ip3,serv3);

  serv3=strtok(NULL," ");
  strcpy(info.server_port3,serv3);


  serv4=strtok(serv4,"\n");

  serv4=strtok(serv4,":");
  strcpy(info.server_ip4,serv4);

  serv4=strtok(NULL," ");
  strcpy(info.server_port4,serv4);


  user1=strtok(user1,"\n");
  strcpy(info.username[0],user1);

  pass1=strtok(pass1,"\n");
  strcpy(info.password[0],pass1);

  user2=strtok(user2,"\n");
  strcpy(info.username[1],user2);

  pass2=strtok(pass2,"\n");
  strcpy(info.password[1],pass2);
}



/*Function to create socket and connect*/
int connect_sock(char* server_ip,int port_no){
    struct sockaddr_in address;
    int sock = 0;
    struct sockaddr_in serv_addr;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\nERROR: SOCKET CREATION ERROR\n");
        return -1;
    }
  
    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port_no);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, server_ip, &serv_addr.sin_addr)<=0)
    {
        printf("\nERROR: INAVLID ADDRESS/ADDRESS NOT SUPPORTED \n");
        return -1;
    }

  
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nERROR: CONNECTION FAILED\n");
        return -1;
    }


    return sock;
}


/*Function to calculate md5sum*/
uint8_t md5sum_calculate(char* filename, char* hash_file){
  FILE* file;
  file=  fopen(filename,"r");
  if(file==NULL){
    printf("\nERROR: FILE TO CALCULATE MD5SUM DOES NOT EXISTS\n");
  }
  fclose(file);

  char md5_hashfile[50]={0};
  char temp_file[50]={0};

  char* temp;
  char hashvalue[32]={0}; 

  memset(md5_hashfile,0,sizeof(md5_hashfile));
  memset(temp_file,0,sizeof(temp_file));
  memset(hashvalue,0,sizeof(hashvalue));


  strcpy(temp_file,filename);
  temp=strtok(temp_file,".");
  strcpy(md5_hashfile, temp);

  strcat(md5_hashfile,"_md5sum.txt");
  strcpy(hash_file,md5_hashfile);


  //calculate hash value
  char hash_array[200]= "";
  snprintf(hash_array,sizeof(hash_array),"md5sum %s > %s | awk '{print $1}'",filename,md5_hashfile);
  system(hash_array); 

  FILE* fp=fopen(md5_hashfile,"r");

  fread(hashvalue,1,32,fp);
  fclose(fp);

  char* last_byte=&hashvalue[31];
  char last[2]={0};

  uint8_t mod4=0;
  
  if(*last_byte >= '0' && *last_byte <= '9'){
    mod4=atoi(last_byte);
  } 
  else{
    sprintf(last,"%x",last_byte);
    mod4=atoi(last) -51;
  }
  
  mod4=mod4 % 4;
  return mod4;

} 


/*Function to encrypt/decrypt*/
void encryption(char* buffer,int len,char* key){
  int i=0;
  int key1;

  int count=strlen(key);

  for(i=0;i<len;i++){
    key1=i%count;
    buffer[i]=buffer[i]^key[key1];
  }
}  


  
int main(int argc, char const *argv[])
{

    int sock[4];
    int i=0;
    int valread=0;
    int num=0;
    FILE* file_send=NULL;
    FILE* file_recv=NULL;
    int size_send=0;
    int size_recv=0;
    char filename_send[20]={0};
    char filename_recv[30]={0};
    char filename_serv[4][20]={0};
    char filesize_send_c[10]={0};
    char filesize_recv_c[20]={0};
    char command;
    char command1[40]={0};
    char subfolder[5]={0};
    char* filename;
    char* sub;
    char username_check[20]={0};
    char password_check[20]={0};
    int user_flag=-1;
    int pass_flag=-1;

    char command_packet[2]={0};
    char verification_packet[40]={0};
    char verification_check[10]={0};
    int ver_count=0;

    char md5sum_check[4][4]={{12,23,34,41},{41,12,23,34},{34,41,12,23},{23,34,41,12}};
    int size_to_send_arr[4]={0};
    int packet_no=0;
    char filename_verify[5]={0};

    char file_get_recv[4][100000]={0};
    uint32_t file_get_size[4]={0};
    int count_work_server=0;

    parse_file();

    while(1){

      user_flag=-1;
      pass_flag=-1;
      ver_count=0;
 
      //wait for command from user    
      printf("\nENTER COMMAND\n");
      memset(command1,0,sizeof(command1));
      memset(subfolder,0,sizeof(subfolder));
      memset(file_get_recv,0,sizeof(file_get_recv));
      memset(file_get_size,0,sizeof(file_get_size));

      gets (command1);
      filename=strtok(command1," ");
      
      //check which command is received
      int ret1=strcmp(command1,"get");
      int ret2=strcmp(command1,"put");
      int ret3=strcmp(command1,"ls");

      if(ret3!=0){
        filename=strtok(NULL, " ");
        sub=strtok(NULL," ");
      }
      else{
        sub=strtok(NULL," ");
      }

      strcpy(subfolder,sub);

      if((ret1 && ret2 && ret3)!=0){
        printf("\nERROR: INVALID COMMAND\n");
        continue;
      }
      
      if(ret2==0){

        //Check if that file exists
        FILE* file_exist=fopen(filename,"r");
        if(file_exist==NULL){
          printf("\nERROR: FILE DOES NOT EXIST. RE-ENTER COMMAND \n");
          continue;
        }
      }

      memset(username_check,0,sizeof(username_check));
      memset(password_check,0,sizeof(password_check));


      //verify username and password at the client
      while((user_flag!=0) || (pass_flag!=0)){
        printf("\nENTER USERNAME\n");
        gets(username_check);

        printf("\nENTER PASSWORD\n");
        gets(password_check);
     
        for(i=0;i<2;i++){
          user_flag=strcmp(username_check,info.username[i]);
          if(user_flag==0){
            break;
          }
        }

        if(user_flag==0){
          pass_flag=strcmp(password_check,info.password[i]);
        }

        if((user_flag!=0) || (pass_flag!=0)){
          printf("\nERROR: INCORRECT USERNAME OR PASSWORD\n");
        }
      } 

      printf("\nUSERNAME/PASSWORD VERIFICATION SUCCESSFUL AT THE CLIENT\n");


      memset(verification_packet,0,sizeof(verification_packet));
      strcpy(verification_packet,info.username[i]);
      strcat(verification_packet," ");
      strcat(verification_packet,info.password[i]);


      count_work_server=0;
      sock[0]=connect_sock(info.server_ip1,atoi(info.server_port1));
      sock[1]=connect_sock(info.server_ip2,atoi(info.server_port2));
      sock[2]=connect_sock(info.server_ip3,atoi(info.server_port3));
      sock[3]=connect_sock(info.server_ip4,atoi(info.server_port4));

      for(i=0;i<4;i++){
        if(sock[i]>0){
          count_work_server++;
        }
      }

      printf("\nWORKING SERVERS = %d\n",count_work_server);

      for(i=0;i<4;i++){
        //Verify username/password from server
        send(sock[i] , (const void*)verification_packet, strlen(verification_packet), MSG_NOSIGNAL );
      }

  

     for(i=0;i<4;i++){
       memset(verification_check,0,sizeof(verification_check));
       //Verify username/password from server by checking ACK, NAK
       valread = read(sock[i] , verification_check, 10);
       int ret_ver=strncmp(verification_check,"ACK",3);
       if(ret_ver==0){
         ver_count++;
       }
     }


     if((ver_count)!=(count_work_server)){
       printf("\nERROR: USERNAME/PASSWORD VERIFICATION FAILED AT THE SERVER\n");
       continue;
     }
     
     printf("\nUSERNAME/PASSWORD VERIFICATION SUCCESSFUL AT THE SERVER\n");


     if(ret1==0){
       command='1';
     }
     else if(ret2==0){
       command='2';
     }
     else{
       command='3';
     }

     memset(command_packet,0,strlen(command_packet));
     strcpy(command_packet,&command);
     
     //sending command
     for(i=0;i<4;i++){
      int val_read =send(sock[i] , (const void*)command_packet,1, MSG_NOSIGNAL );
     }          

     sleep(1); 

     //PUT COMMAND
     if(ret2==0){

       printf("\nPUT COMMAND\n");
       

       //calculate md5sum
       char hashfile[50]={0};
       memset(hashfile,0,sizeof(hashfile));
       uint8_t mod4=md5sum_calculate(filename,hashfile);

       printf("\nHASH VALUE STORED IN FILE %s\n",hashfile);

       memset(filename_send,0,sizeof(filename_send));
       strcpy(filename_send,filename);

       file_send=fopen(filename_send,"rb");

       fseek(file_send,0,SEEK_END);
       size_send=ftell(file_send);
       fseek(file_send,0,SEEK_SET);

       for(i=0;i<4;i++){
         memset(filename_serv[i],0,sizeof(filename_serv[i]));
       }

    
       strcpy(filename_serv[0],filename_send);
       strcat(filename_serv[0],".1 ");
       strcat(filename_serv[0],subfolder);


       strcpy(filename_serv[1],filename_send);
       strcat(filename_serv[1],".2 ");
       strcat(filename_serv[1],subfolder);
    
 
       strcpy(filename_serv[2],filename_send);
       strcat(filename_serv[2],".3 ");
       strcat(filename_serv[2],subfolder);


       strcpy(filename_serv[3],filename_send);
       strcat(filename_serv[3],".4 ");
       strcat(filename_serv[3],subfolder);


       //calculate chunk size
       memset(size_to_send_arr,0,sizeof(size_to_send_arr));
       for(i=0;i<3;i++){
         size_to_send_arr[i]=size_send/4;
       }
       size_to_send_arr[3]=size_send/4 + (size_send % 4);

       //store chunk values
       char file_chunk_store[4][size_to_send_arr[3]];


       for(i=0;i<4;i++){
         memset(file_chunk_store[i],0,sizeof(file_chunk_store[i]));
       }

       
       for(i=0;i<4;i++){
         fread(file_chunk_store[i],1,size_to_send_arr[i],file_send);
         //----------encryption--------
         encryption(file_chunk_store[i],size_to_send_arr[i],password_check);
       }
       
       fclose(file_send);


       for(i=0;i<4;i++){

         packet_no=((md5sum_check[mod4][i])/10)-1;

         send(sock[i] , (const void*)filename_serv[packet_no],strlen(filename_serv[packet_no]), MSG_NOSIGNAL );
         sleep(1);
 
         //send size_to_send next
         memset(filesize_send_c,0,sizeof(filesize_send_c));
         sprintf(filesize_send_c,"%d",size_to_send_arr[packet_no]);

         send(sock[i] , (const void*)filesize_send_c, strlen(filesize_send_c), MSG_NOSIGNAL );

         sleep(1);
       
         //send file data

         send(sock[i] ,(const void*) file_chunk_store[packet_no], size_to_send_arr[packet_no], MSG_NOSIGNAL);


         sleep(1);
         //second file
         packet_no=((md5sum_check[mod4][i])%10)-1;

         //file name
         send(sock[i] , (const void*)filename_serv[packet_no],strlen(filename_serv[packet_no]), MSG_NOSIGNAL );
         sleep(1);

         //send size_to_send next
         memset(filesize_send_c,0,sizeof(filesize_send_c));
         
         sprintf(filesize_send_c,"%d",size_to_send_arr[packet_no]);

         send(sock[i] , (const void*)filesize_send_c, strlen(filesize_send_c), MSG_NOSIGNAL );

         sleep(1);

         //send file data

         send(sock[i] ,(const void*) file_chunk_store[packet_no], size_to_send_arr[packet_no], MSG_NOSIGNAL );
         
       }

       printf("\nFILE SENT CLIENT\n");
    }

    //GET COMMAND
    else if(ret1==0){

      printf("\nGET COMMAND\n");
      memset(filename_send,0,sizeof(filename_send));
      strcpy(filename_send,filename);
      strcat(filename_send," ");
      strcat(filename_send, subfolder);


      char filename_array[4][20]={0};

      for(i=0;i<4;i++){
        memset(filename_array[i],0,sizeof(filename_array[i]));
        strcpy(filename_array[i],filename);        
      }

      strcat(filename_array[0],".1");
      strcat(filename_array[1],".2");
      strcat(filename_array[2],".3");
      strcat(filename_array[3],".4");     


      uint8_t count_filepart[4]={0};
      memset(count_filepart,0,sizeof(count_filepart));
      uint8_t j=0;

      sleep(2);

      for(i=0;i<4;i++){

         //send filename first
         send(sock[i] , (const void*)filename_send,strlen(filename_send), MSG_NOSIGNAL );

    
         //PART1
         //recv part name, check if you need that part and correspondingly send ack or nak
         memset(filename_recv,0,sizeof(filename_recv));

         valread = read(sock[i], filename_recv, 20);

         for(j=0;j<4;j++){
           int8_t ret=-1;
           ret= strncmp(filename_recv, filename_array[j],valread);
           if(ret==0){
             break;
           }
                  
         }

         if(j>3){
//           printf("\nERROR: FILE NOT RECOGNIZED\n");
         }

         memset(filename_verify,0,sizeof(filename_verify));
         memset(filesize_recv_c,0,sizeof(filesize_recv_c));

         //check if you neeed that file part and send corresponding ACK or NAK
         if(count_filepart[j]>0){
           strcpy(filename_verify,"NAK");
           send(sock[i] , (const void*)filename_verify,strlen(filename_verify), MSG_NOSIGNAL );
         }
         else{
           count_filepart[j]++;
           strcpy(filename_verify,"ACK");
           send(sock[i] , (const void*)filename_verify,strlen(filename_verify), MSG_NOSIGNAL );

           //Receive filesize
           valread = read(sock[i], filesize_recv_c, 20);
           uint32_t filesize_recv=atoi(filesize_recv_c);

           char file_get[filesize_recv];


           
           memset(file_get,0,sizeof(file_get));
           
           //Receive the part data
           valread = read(sock[i], file_get_recv[j], filesize_recv);
           file_get_size[j]=valread;

           //-------decryption-------
           encryption(file_get_recv[j],filesize_recv,password_check);
           

           printf("\nFILE RECEIVED %s\n OF SIZE %d\n",filename_recv,filesize_recv);
         }



         //PART2
         memset(filename_recv,0,sizeof(filename_recv));
         memset(filesize_recv_c,0,sizeof(filesize_recv_c));

         //Receive the filename
         valread = read(sock[i], filename_recv, 20);

         for(j=0;j<4;j++){
           int8_t ret1 =-1;
           ret1 = strncmp(filename_recv, filename_array[j],valread);
         
           if(ret1==0){
             break;
           }
                  
         }

         if(j>3){
           printf("\nERROR: FILE NOT RECOGNIZED\n");
         }

         memset(filename_verify,0,sizeof(filename_verify));

         //Check if you need that file part and send corresponding ACK or NAK
         if(count_filepart[j]>0){
           strcpy(filename_verify,"NAK");
           send(sock[i] , (const void*)filename_verify,strlen(filename_verify), MSG_NOSIGNAL );
         }
         else{
           count_filepart[j]++;
           strcpy(filename_verify,"ACK");
           send(sock[i] , (const void*)filename_verify,strlen(filename_verify), MSG_NOSIGNAL );

           //Receive the file size
           valread = read(sock[i], filesize_recv_c, 20);

           uint32_t filesize_recv=atoi(filesize_recv_c);


           char file_get[filesize_recv];
         
           memset(file_get,0,sizeof(file_get));
           
           //Receive the part data
           valread = read(sock[i], file_get_recv[j], filesize_recv);

           file_get_size[j]=valread;

           //------encryption--------------
           encryption(file_get_recv[j],filesize_recv,password_check);


           printf("\nFILE RECEIVED %s OF SIZE %d\n",filename_recv,filesize_recv);

         }

           

      }

      char received_array[50]={0};
      memset(received_array,0,sizeof(received_array));
      strcpy(received_array,"Received_");
      strcat(received_array,filename);

      //Write the file received to a file
      FILE* file_write=fopen(received_array,"wb");
      for(j=0;j<4;j++){

        fwrite(file_get_recv[j],1,file_get_size[j],file_write);
      }
      fclose(file_write);

    }
  
    //ls command
    else{
      char files_array[30][300]={0};
      
      char buffer[500]={0};

      printf("\nLIST COMMAND\n");

      char list_files[4][400];

      uint8_t m=0;

      for(m=0;m<4;m++){
        memset(list_files[m],0,sizeof(list_files[m]));
      }

      FILE* f1=fopen("list_store1","w");
      
      for(m=0;m<4;m++){
      
        //sending subfolder
        send(sock[m] , (const void*)subfolder,strlen(subfolder), MSG_NOSIGNAL );


        //recv filesize
        int num=0;
        char size_to_be_received_string[50]={0};

        valread = read(sock[m], size_to_be_received_string, sizeof(size_to_be_received_string));

        int size_to_be_received=atoi(size_to_be_received_string);


        int size_left=size_to_be_received;
        
        int z=0;

        //receive files
        while(size_left){
          num=read(sock[m], buffer,1);
          size_left-=num;
          list_files[m][z]=buffer[0];
          z++;
        }

        //write them to a file
        fwrite(list_files[m],1,strlen(list_files[m]),f1);
        
      }

      char file_end[]="\n.zzEOF.end.1";
      fwrite(file_end,1,strlen(file_end),f1);

      fclose(f1);  
      printf("\nRECEIVED THE LIST\n");

      bzero(buffer,500);

      char sys_call[100]={0};
      
      //sort the first file and store it in another file
      strcpy(sys_call,"sort list_store1 | uniq > list_store2");

      system(sys_call);

      bzero(sys_call,100);

      //remove the first file
      strcpy(sys_call,"rm list_store1");
      system(sys_call);
      
      //open the sorted file and check if the file is complete
      FILE* f2=fopen("list_store2","r");
      char temp_file_server[200]={0};

      int a=0;
      int b=0;


      bzero(buffer,500);

      while(!feof(f2)){
        fgets(buffer,200,f2);
        bzero(temp_file_server,200);
        if(buffer[0]=='.'){
          char* temp=strtok((char*)&buffer[1],".");
          sprintf(temp_file_server,"%s",temp);

          if(b==0){
            bzero(files_array[b],300);
            strncpy(files_array[b],temp_file_server,strlen(temp_file_server));
            b++;

          }

          else if(strncmp(temp_file_server,files_array[b-1],strlen(temp_file_server))==0){
            a++;
            if(a==3){
              printf("\n%s\n",files_array[b-1]);
              a=0;
            }

          }

          else{
            if(a!=0){
              printf("\n%s incomplete\n",files_array[b-1]);
            }
            bzero(files_array[b],300);
            strncpy(files_array[b],temp_file_server,strlen(temp_file_server));
            b++;
            a=0;
          }

        }


      }

      fclose(f2);

      bzero(sys_call,100);
      strcpy(sys_call,"rm list_store2");

      system(sys_call);
 
    }
  } 

  return 0;
}

