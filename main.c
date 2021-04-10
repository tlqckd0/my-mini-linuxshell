#include <stdio.h>
#include <string.h>
#include <unistd.h> 
#include <sys/wait.h>
#include <time.h>
#include <stdlib.h>

#define MAX_LINE 80 
#define MAX_HISTORY 10

int main(void){
    
    int should_run = 1;
    int cnt = 0;
    int status;
    int history_cnt=0;

    int cuncurrent;

    pid_t pid;
    char history[MAX_HISTORY+1][MAX_LINE];
    char input[MAX_LINE], his[MAX_LINE];
    char *args[MAX_LINE/2+1];


    while(should_run){
        //출력
        printf("OS > ");    
        fflush(stdout);     
        fflush(stdin);
        // 입력받는부분
        fgets(input,MAX_LINE,stdin);  
        if(strcmp(input,"\n") == 0){
            continue;
        } 

       
        cnt = 0;
        cuncurrent = -1;
        input[strlen(input)-1] = '\0'; //개행문자 날려버림
        strcpy(his,input);
        args[cnt] = strtok(input," ");
        while (args[cnt] != NULL){
            if(strcmp(args[cnt],"&")== 0){
                cuncurrent = cnt;
            }
            args[++cnt] = strtok(NULL, " ");  
        }

         //history 출력
        if(strcmp(args[0],"history") == 0){
            for(int i=history_cnt-1;i>=0;i--){
                printf("%d %s\n",i+1, history[i]);
            }
            continue;
        }

        //직전 명령어 수행
        if(strcmp(args[0],"!!") == 0){
            if(history > 0){
                strcpy(input, history[0]);
            }else{
                continue;
            }
        }else if(strcmp(args[0],"!") == 0){
            int num = atoi(args[1]);
            if(num == 0){
                perror("invalid history num");
                continue;
            }
            strcpy(input,history[num-1]);
            cnt = 0;
            cuncurrent = -1;
            strcpy(his,input);
            args[cnt] = strtok(input," ");
            while (args[cnt] != NULL){
                if(strcmp(args[cnt],"&")== 0){
                    cuncurrent = cnt;
                }
                args[++cnt] = strtok(NULL, " ");  
            }
        }else{
            //한칸씩 위로 밀어내기
            for(int i = history_cnt; i > 0;i--){
                strcpy(history[i], history[i-1]);
            }
            strcpy(history[0], his);
            if(history_cnt <= 10)history_cnt++;
        }


        
        //종료하는것
        if(strcmp(args[0],"exit") == 0){
            puts("logout");
            should_run = 0;
        }else{            
        //실행부분
            if((pid = fork()) < 0){
                //자식 프로세스 생성 실패..
                perror("fork error");
            }
            else if(pid == 0){
                
                if(cuncurrent != -1){

                    pid_t pid_c = fork();
                    int next_cmd = cuncurrent+1;
                    
                    if(pid_c < 0){
                        perror("fork error");
                    }

                    if(pid_c == 0){//앞에꺼실행(main->자식->자식)
                        
                        if(cuncurrent == 1){
                            execlp(args[0],args[0],(char*)0);
                        }else if(cuncurrent == 2){
                            execlp(args[0],args[0],args[1],(char*)0);
                        }                        
                    }else{//뒤에꺼도 실행(main-> 자식)
                        waitpid(pid_c, &status, 0);
                        if(cnt - cuncurrent == 1){
                            execlp(args[next_cmd],args[next_cmd],(char*)0);
                        }else if(cnt - cuncurrent== 2){
                            execlp(args[next_cmd],args[next_cmd],args[next_cmd+1],(char*)0);
                        }
                    }
                }else{
                    //그냥 하나짜리 실행
                    if(cnt == 1){
                        execlp(args[0],args[0],(char*)0);
                    }else if(cnt == 2){
                        execlp(args[0],args[0],args[1],(char*)0);
                    }                
                }
                perror("false");
            }

            //기다려주자.
            waitpid(pid, &status, 0);
        }


    }
    return 0;
}