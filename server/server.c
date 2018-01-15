//Includes
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <sys/dir.h>


#define BUFFER_SIZE 1024

//Structure to store username and password from the conf file
typedef struct user_info_t{
  char username[2][20];
  char password[2][20];
}user_info;

user_info info;

//Function to read username password from conf file
void parse_file(){
  char buffer[BUFFER_SIZE];

  char username1[11]="Username1:";
  char password1[11]="Password1:";
  
  char username2[11]="Username2:";
  char password2[11]="Password2:";


  int size=0;
  
  char* user1;
  char* pass1;


  char* user2;
  char* pass2;


  FILE* conf_file;
  conf_file=fopen("server.conf","r");
  
  fseek(conf_file,0,SEEK_END);
  size=ftell(conf_file);
  fseek(conf_file,0,SEEK_SET);

  fread(buffer,1,size,conf_file);

  fclose(conf_file);

  user1=strstr(buffer,username1);
  user1=user1 + strlen(username1)+1;


  pass1=strstr(buffer,password1);
  pass1=pass1 + strlen(password1)+1;


  user2=strstr(buffer,username2);
  user2=user2 + strlen(username2)+1;


  pass2=strstr(buffer,password2);
  pass2=pass2 + strlen(password2)+1;


  user1=strtok(user1,"\n");
  strcpy(info.username[0],user1);

  pass1=strtok(pass1,"\n");
  strcpy(info.password[0],pass1);

  user2=strtok(user2,"\n");
  strcpy(info.username[1],user2);

  pass2=strtok(pass2,"\n");
  strcpy(info.password[1],pass2);

}


//Function to check is a file exists
int file_check(const struct direct *entry){
  if((strcmp(entry->d_name,"..")==0) || (strcmp(entry->d_name,".")==0)){
    return 0;
  }
  else{
    return 1;
  }
}


int main(int argc, char const *argv[]){

  int port_no=atoi(argv[1]);

  int server_fd, new_socket, valread;
  struct sockaddr_in address;
  int opt = 1;
  int addrlen = sizeof(address);


  FILE* file_send=NULL;
  FILE* file_recv=NULL;
  int size;    
  char filename_recv[50]={0};
  char filename_sub_recv[50]={0};
  char filename_send[20]={0};
  char size_recv[20]={0};
  char size_send[20]={0};
  int filesize_recv=0;
  int filesize_send=0;
  char username_recv[20]={0};
  char* user_recv;
  char* pass_recv;
  char password_recv[20]={0};
  char verification_packet[40]={0};
  int i=0;
  char subfolder[20]={0};
  int user_cmp=-1;
  int pass_cmp=-1;
  char verification_ack[5]={0};
  char command_packet[2]={0};
  char create_path[50]={0};
  char create_path1[50]={0};
  char filename_verify[5]={0};
  char filesize_send_c[20]={0};

  parse_file();



  // Creating socket file descriptor
  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0){
    perror("SOCKET FAILED");
    exit(EXIT_FAILURE);
  }
  
  
  // Forcefully attaching socket to the port 8080
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,&opt, sizeof(opt))){
    perror("SETSOCKOPT FAILURE");
    exit(EXIT_FAILURE);
  }
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons( port_no );
      
  // Forcefully attaching socket to the port 8080
  if (bind(server_fd, (struct sockaddr *)&address, 
                                 sizeof(address))<0)
  {
    perror("BIND FAILED");
    exit(EXIT_FAILURE);
  }

  if (listen(server_fd, 3) < 0)
  {
    perror("LISTEN FAILED");
    exit(EXIT_FAILURE);
  }


  while(1){
    printf("\nWAITING FOR USERNAME/PASSWORD\n");
    user_cmp=-1;
    pass_cmp=-1;
    memset(verification_packet,0,sizeof(verification_packet));
    memset(username_recv,0,sizeof(username_recv));
    memset(password_recv,0,sizeof(password_recv));
    memset(subfolder,0,sizeof(subfolder));
    memset(filename_recv,0,sizeof(filename_recv));
    memset(create_path,0,sizeof(create_path));
    memset(create_path1,0,sizeof(create_path1));

    //Accept connection
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, 
                       (socklen_t*)&addrlen))<0){
       perror("ACCEPT FAILURE");
       exit(EXIT_FAILURE);
    }

    //Multithreaded, create a new process for every request
    int child=fork();

    if(child==0){
      printf("\nNEW CHILD PROCESS CREATED\n");
     
      close(server_fd);

      //Read the username and password and check if they are valid
      valread=read(new_socket ,(void*)verification_packet, 40);

      user_recv=strtok(verification_packet," ");
      pass_recv=strtok(NULL," ");

      strcpy(username_recv,user_recv);
      strcpy(password_recv,pass_recv);


      for(i=0;i<2;i++){
        user_cmp=strcmp(username_recv,info.username[i]);
        if(user_cmp==0){
          break;
        }
      }

      if(user_cmp==0){
        pass_cmp=strcmp(password_recv,info.password[i]);
      }

      memset(verification_ack,0,sizeof(verification_ack));
    
      if(user_cmp!=0 || pass_cmp!=0){
        strcpy(verification_ack,"NAK");
        printf("\nERROR: USERNAME/PASSWORD VERIFICATION FAILED AT THE SERVER \n");
        send(new_socket , (const void*)verification_ack, strlen(verification_ack), 0 );
        continue;
      }
      else{
        strcpy(verification_ack,"ACK");
        printf("\nUSERNAME/PASSWORD VERIFICATION SUCCESSFUL AT THE SERVER\n");
        send(new_socket , (const void*)verification_ack, strlen(verification_ack), 0 );
      }
      
      printf("\nWAITING FOR COMMAND\n");
    
      memset(command_packet,0,sizeof(command_packet));

      valread=read(new_socket ,(void*)command_packet, 2); 
  

      //PUT COMMAND 
      if(command_packet[0]=='2'){
        printf("\nPUT COMMAND RECEIVED\n");
        char* file_r;
        char* sub_r; 
   
        memset(filename_sub_recv,0,sizeof(filename_sub_recv));
        memset(create_path1,0,sizeof(create_path1));
        memset(create_path,0,sizeof(create_path));

        //Read file name and subfolder name
        valread=read(new_socket ,(void*)filename_sub_recv, 20);
        file_r=strtok(filename_sub_recv," ");
        sub_r=strtok(NULL," ");


        //Receive the filesize  
        memset(size_recv,0,sizeof(size_recv));
        valread=read(new_socket ,(void*)size_recv, 10);
        filesize_recv=atoi(size_recv);
        printf("\nFILESIZE OF PART = %d\n",filesize_recv);

        char chunk_store[filesize_recv];
        memset(chunk_store,0,sizeof(chunk_store));
        memset(filename_recv,0,sizeof(filename_recv));


        //create path for a particular file depending on the port number and subfolder
        if(port_no==10001){
          strcpy(filename_recv,"DFS1/");
          system("mkdir -p DFS1");
          strcpy(create_path,"mkdir -p DFS1/");
        }
        else if(port_no==10002){
          strcpy(filename_recv,"DFS2/");
          system("mkdir -p DFS2");
          strcpy(create_path,"mkdir -p DFS2/");
        }
        else if(port_no==10003){
          strcpy(filename_recv,"DFS3/");
          system("mkdir -p DFS3");
          strcpy(create_path,"mkdir -p DFS3/");
        }
        else{
          strcpy(filename_recv,"DFS4/");
          system("mkdir -p DFS4");
          strcpy(create_path,"mkdir -p DFS4/");
        }

        strcat(create_path,username_recv);
        system(create_path);
        strcpy(create_path1,create_path);
        strcat(create_path1,sub_r);
        system(create_path1);
      

     
        strcat(filename_recv,username_recv);
        strcat(filename_recv,sub_r);
        strcat(filename_recv,".");
        strcat(filename_recv,file_r);

        //Open file and read the file data and write data to the file
        file_recv=fopen(filename_recv,"wb");
        valread=read(new_socket ,(void*)chunk_store, filesize_recv);


        fwrite(chunk_store,1,valread,file_recv);

        fclose(file_recv);

        printf("\nFILE 1st PART RECEIVED\n");



        //Receive second file
        memset(filename_sub_recv,0,sizeof(filename_sub_recv));
        memset(create_path,0,sizeof(create_path));
        memset(create_path1,0,sizeof(create_path1));

        //Read the filename and subfolder name
        valread=read(new_socket ,(void*)filename_sub_recv, 20);
        file_r=strtok(filename_sub_recv," ");
        sub_r=strtok(NULL," ");

        //Read the filesize
        memset(size_recv,0,sizeof(size_recv));
        valread=read(new_socket ,(void*)size_recv, 10);
        filesize_recv=atoi(size_recv);
        printf("\nFILESIZE OF PART = %d\n",filesize_recv);

        char chunk_store1[filesize_recv];
        memset(chunk_store1,0,sizeof(chunk_store1));
        memset(filename_recv,0,sizeof(filename_recv));


        //Create path depending of the filename and subfolder name
        if(port_no==10001){
          strcpy(filename_recv,"DFS1/");
          system("mkdir -p DFS1");
          strcpy(create_path,"mkdir -p DFS1/");
        }
        else if(port_no==10002){
          strcpy(filename_recv,"DFS2/");
          system("mkdir -p DFS2");
          strcpy(create_path,"mkdir -p DFS2/");
        }
        else if(port_no==10003){
          strcpy(filename_recv,"DFS3/");
          system("mkdir -p DFS3");
          strcpy(create_path,"mkdir -p DFS3/");
        }
        else{
          strcpy(filename_recv,"DFS4/");
          system("mkdir -p DFS4");
          strcpy(create_path,"mkdir -p DFS4/");
        }
    
        strcat(create_path,username_recv);
        system(create_path);
        strcpy(create_path1,create_path);
        strcat(create_path1,sub_r);
        system(create_path1);


        strcat(filename_recv,username_recv);
        strcat(filename_recv,sub_r);
        strcat(filename_recv,".");
        strcat(filename_recv,file_r);


        //Open file, read data and write that to the file
        file_recv=fopen(filename_recv,"wb");
        valread=read(new_socket ,(void*)chunk_store1, filesize_recv);


        fwrite(chunk_store1,1,valread,file_recv);

        fclose(file_recv);


        printf("\nFILE 2nd PART RECEIVED\n");

      }


      //GET command
      else if(command_packet[0]=='1'){
        printf("\nGET COMMAND RECEIVED\n");
        char* file_r_1;
        char* sub_r_1;
      
        char filename_sub_recv_1[20]={0};

        memset(filename_sub_recv_1,0,sizeof(filename_sub_recv_1));

        //Receive the filename and subfolder name
        valread=recv(new_socket ,(void*)filename_sub_recv_1, 20,0);
        printf("\nFILENAME + SUBFOLDER = %s\n",filename_sub_recv_1);
    
      
        file_r_1=strtok(filename_sub_recv_1," ");
        sub_r_1=strtok(NULL," ");


        char file_checkarray[4][100]={0};

        char file_array[4][100]={0};
      

        uint8_t j=0;

        for(j=0;j<4;j++){
          memset(file_checkarray[j],0,sizeof(file_checkarray[j]));
          memset(file_array[j],0,sizeof(file_array[j]));
        }

        //Create path array depending on the port number and subfolder
        if(port_no==10001){
          for(j=0;j<4;j++){
            strcpy(file_checkarray[j],"DFS1/");
          }
        }
        else if(port_no==10002){
          for(j=0;j<4;j++){
            strcpy(file_checkarray[j],"DFS2/");
          }
        }
        else if(port_no==10003){
          for(j=0;j<4;j++){
            strcpy(file_checkarray[j],"DFS3/");
          }
        }
        else{
          for(j=0;j<4;j++){
            strcpy(file_checkarray[j],"DFS4/");
          }
        }

        for(j=0;j<4;j++){
          strcat(file_checkarray[j],username_recv);
          strcat(file_checkarray[j],sub_r_1);
          strcat(file_checkarray[j],".");
          strcat(file_checkarray[j],file_r_1);
          strcat(file_array[j],file_r_1);
        }


        strcat(file_checkarray[0],".1");
        strcat(file_checkarray[1],".2");
        strcat(file_checkarray[2],".3");
        strcat(file_checkarray[3],".4");


        strcat(file_array[0],".1");
        strcat(file_array[1],".2");
        strcat(file_array[2],".3");
        strcat(file_array[3],".4");
  
      
        for(j=0;j<4;j++){
          FILE* partfile=fopen(file_checkarray[j],"rb");

          if (partfile==NULL){
           // printf("\n%s not found\n",file_checkarray[j]);
          }

          else{
            send(new_socket , (const void*)file_array[j], strlen(file_array[j]), 0 ); 
         
            memset(filename_verify,0,sizeof(filename_verify));

            //Send the filename and check if that part is required or not
            valread=read(new_socket ,(void*)filename_verify, 5);

            int8_t ret1=strcmp(filename_verify,"ACK");
            if(ret1==0){
              fseek(partfile,0,SEEK_END);
              uint32_t size_send=ftell(partfile);
              fseek(partfile,0,SEEK_SET);
              memset(filesize_send_c,0,sizeof(filesize_send_c));
              sprintf(filesize_send_c,"%d",size_send);
        
              printf("\nSENDING FILE %s of FILESIZE %d\n",file_checkarray[j],size_send); 

              //If that part is required then send file size
              send(new_socket , (const void*)filesize_send_c, strlen(filesize_send_c), 0 );
            
              sleep(1);
          
              char file_content[size_send];
              memset(file_content,0,sizeof(file_content));

              //Read the file content
              fread(file_content,1,size_send,partfile);

              fclose(partfile);

              //send the file data
              send(new_socket, (const void*)file_content, size_send, 0 );


            }
            else{
              fclose(partfile);
            }

          }
          sleep(1);       
        }
      }
      //LS command
      else{
        printf("\nLS COMMAND RECEIVED\n");
        char subfolder_ls[10]={0};

        //read the subfolder name
        valread=read(new_socket ,(void*)subfolder_ls, 10);

        char filename_recv_ls[100]={0};

        if(port_no==10001){
          strcpy(filename_recv_ls,"DFS1/");
        }
        else if(port_no==10002){
          strcpy(filename_recv_ls,"DFS2/");
        }
        else if(port_no==10003){
          strcpy(filename_recv_ls,"DFS3/");
        }
        else{
          strcpy(filename_recv_ls,"DFS4/");
        }

        strcat(filename_recv_ls,username_recv);
   
        strcat(filename_recv_ls,subfolder_ls);
        
        printf("\nSUBFOLDER PATH = %s\n",filename_recv_ls);

        int count=0;
        
        int i=0;
        struct direct **file_structure;
        char list_files[500]={0};
        
        count=scandir(filename_recv_ls,&file_structure,file_check,alphasort);


        if(count<=0){
          printf("\nERROR: NO FILES IN THE SERVER FOR THE MENTIONED SUBDIRECTORY\n");
          exit(1);
        }

        for(i=1;i<=count;i++){
          sprintf(list_files + strlen(list_files), "%s\n",file_structure[i-1]->d_name);
        }
        sprintf(list_files + strlen(list_files),"\n");

        int num= strlen(list_files);
        char list_files_length_string[10]={0};

        memset(list_files_length_string,'\0',sizeof(list_files_length_string));

        sprintf(list_files_length_string,"%d",num);

        //send the list size
        send(new_socket , (const void*)list_files_length_string, sizeof(list_files_length_string), 0 );



        sleep(1);

        //send the list files
        send(new_socket , (const void*)list_files, strlen(list_files), 0 );

      }

      close(new_socket);
      exit(1);
    }

    else{
     close(new_socket);

    }
  }

  return 0;
}
