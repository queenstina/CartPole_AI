#ifndef __GAMEMANGER_H__
#define __GAMEMANGER_H__
#include "Board.h"
#include "Agent.h"
#include <linux/input.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <string>

enum : char {RELEASED, PRESSED, REPEATED};

std::vector<int> topology(int n, int m){
	return std::vector<int>({n*m+4,n*m/2,1});
}

template<int n, int m>
class GameManager{
private:
	int kb;
	Agent<n,m> ai;
	input_event ev;
	Board<n,m> board;
	std::string who;
public:
	GameManager(std::string who):who(who),ai(topology(n,m)){
		for(auto& c : who){
			c = std::tolower(c);
		}
		if(who == "kb"){
			std::cout << "KB" << endl;
			const char* dev = "/dev/input/by-path/platform-i8042-serio-0-event-kbd";
			kb = open(dev,O_RDONLY); //read keyboard
			if(kb == -1){
				fprintf(stderr,"Cannot open device %s: %s.\n", dev,strerror(errno));
				close(kb);
				throw("CANNOT OPEN DEVICE");
			}
		}else if (who == "ai"){
			std::cout << "AI" << endl;
			//AI SETUP CODE HERE
		}
		board.print();
	}
	~GameManager(){
		if(who == "kb")
			close(kb);
	}


	bool CMDread(DIR& dir){
		if(who == "kb")
			return KBread(dir);
		else if (who == "ai")
			return AIread(dir);
	}
	bool KBread(DIR& dir){
		dir = X;
		ssize_t bytes = read(kb,&ev,sizeof(ev));
		
		if(bytes == (ssize_t)-1){
			if(errno == EINTR){
				return true;
			}
			else{
				return false;
			}
		}else if(bytes != sizeof(ev)){
			errno = EIO;
			return false;
		}

		if(ev.type == EV_KEY && ev.value == PRESSED){
			switch(ev.code){
				case 0x006a: //RIGHT
					dir = R;
					break;
				case 0x0067: //UP
					dir = U;
					break;
				case 0x0069: //LEFT
					dir = L;
					break;
				case 0x006c: //DOWN
					dir = D;
					break;
				default:
					break;
			}
		}
		return true;
	}
	bool AIread(DIR& dir){
		dir = ai.getNext(board);
		return true;
	}
	void run(){
		DIR dir = X;
		while(CMDread(dir)){
			if(dir == X || board.end()){
				board = Board<n,m>();
				//new episode
			}else{
				//UPDATE Q-Value
				int r = board.next(dir); //immediate reward
				float mv = ai.getMax(board);//max of "next" state(= this state now)
				ai.update(dir,r,mv);
				//not yet implemented
				
				board.print();
				cout << endl;
			}
		}
	}
};
#endif