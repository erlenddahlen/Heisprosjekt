#include "queue.h"

int order_typeEqual(struct order_type order_1, struct order_type order_2){
  if (order_1.dir==order_2.dir && order_1.floor==order_2.floor){
    return 1;
  } else{
    return 0;
  }
}

int external_orderEqual(struct external_order ext_order_1,struct external_order ext_order_2){
  if (order_typeEqual(ext_order_1.order,ext_order_2.order) && (ext_order_1.floorstop==ext_order_2.floorstop)){
    return 1;
  } else{
    return 0;
  }
}

void queueInit()
{
  int i=0;
  while(queue[i].valid){
  queue[i].order.dir=0; queue[i].order.floor=0;queue[i].valid=0;memset(queue[i].floorstop,0,sizeof(queue[i].floorstop));
  i++;
  }
}
void addExternalOrder(struct order_type neworder)
{
  if (neworder.dir==queue[0].order.dir){
    if (neworder.dir==UP && neworder.floor<queue[0].order.floor &&currentFloor<neworder.floor){
    	queue[0].floorstop[neworder.floor-1]=1;
      return;
    }
    else if (neworder.dir==DOWN && neworder.floor>queue[0].order.floor && currentFloor>neworder.floor){
      queue[0].floorstop[neworder.floor-1]=1;
      return;
    }
  }
  for (int i=0;i<10;i++){
  	if (!queue[i].valid){
      queue[i].order=neworder;
      queue[i].floorstop[neworder.floor-1]=1;
      queue[i].valid=1;
      return;
    }
    if(neworder.dir==queue[i].order.dir && neworder.floor==queue[i].order.floor){ //orderEqual(neworder,queue[i])
      queue[i].floorstop[neworder.floor-1]=1;
      return;
    }
	}
}


void addInternalOrder(int floor) {
  if((floor==currentFloor)&&(floorValid>-1)){
    stopElev();
    return;
  }
  if ((io_read_bit(MOTORDIR)==1 && floor>=currentFloor)&&(queue[0].valid)) {
    int index=0;
    int temp_valid=1;
    while((index<9)&&(temp_valid==1)){
      index++;
      if(queue[index].valid==0){
        if (queue[index-1].order.floor==floor){
          temp_valid=0;
        }else{
          queue[index].valid=1;
          queue[index].floorstop[floor-1]=1;
          queue[index].order.floor=floor;
          queue[index].order.dir=NONE;
          temp_valid=0;
        }
      }
    }
    return;
  }else if((io_read_bit(MOTORDIR)==0 && floor<=currentFloor)&&(queue[0].valid)){
    int index=0;
    int temp_valid=1;
    while((index<9)&&(temp_valid==1)){
      index++;
      if(queue[index].valid==0){
        if (queue[index-1].order.floor==floor){
        	temp_valid=0;
        }else{
          queue[index].valid=1;
          queue[index].floorstop[floor-1]=1;
          queue[index].order.floor=floor;
          queue[index].order.dir=NONE;
          temp_valid=0;
      	}
    	}
    }
    return;
  }else{
  		queue[0].floorstop[floor-1]=1;
      if((queue[0].order.dir==UP) && (floor>queue[0].order.floor)){
        queue[0].order.floor=floor;
      }else if((queue[0].order.dir==DOWN) && (floor<queue[0].order.floor)){
        queue[0].order.floor=floor;
      }
			if(!queue[0].valid){
				queue[0].valid=1;
				queue[0].order.floor=floor;
				if(floor>currentFloor){
					queue[0].order.dir=UP;
				}else if(floor<currentFloor){
					queue[0].order.dir=DOWN;
				}
			}
		}
}

int orderFinished(){
  return (queue[0].floorstop[0]==0 && queue[0].floorstop[1]==0 && queue[0].floorstop[2]==0 && queue[0].floorstop[3]==0);
}

void removeOrder(int index){
  int i=index;
  int counter=i+1;
  while(queue[counter].valid){
    queue[counter-1].order.dir=queue[counter].order.dir;
    queue[counter-1].order.floor=queue[counter].order.floor;
    for (int k=0;k<4;k++){
      queue[counter-1].floorstop[k]=queue[counter].floorstop[k];
    }
    queue[counter-1].valid=queue[counter].valid;
    i=counter;
		counter++;
  }
  queue[i].order.dir=0; queue[i].order.floor=0;queue[i].valid=0;memset(queue[i].floorstop,0,sizeof(queue[i].floorstop));
}

void newOrder(){
  struct order_type new_order;
  for (int floor=0;floor<4;floor++){
    for (elev_button_type_t button=BUTTON_CALL_UP;button<=BUTTON_COMMAND;button++){
      if((button==BUTTON_CALL_DOWN&&floor==0)||(button==BUTTON_CALL_UP&&floor==3)){
        continue;
      }
			if(elev_get_button_signal(button,floor)){
				elev_set_button_lamp(button,floor,1);
        new_order.floor=floor+1;
        if(button==0){
          new_order.dir=UP;
          addExternalOrder(new_order);
        }else if(button==1){
          new_order.dir=DOWN;
          addExternalOrder(new_order);
        }else{
          new_order.dir=NONE;
          addInternalOrder(new_order.floor);
        }
      }
    }
  }
}


void optimizeQueue(){
	if(queue[0].order.dir==NONE){
  	if(queue[0].order.floor==4){
    	queue[0].order.dir=DOWN;
  	}else if(queue[0].order.floor==1){
    	queue[0].order.dir=UP;
  	}else if(queue[0].order.floor>currentFloor){
    	queue[0].order.dir=UP;
  	}else if(queue[0].order.floor<currentFloor){
    	queue[0].order.dir=DOWN;
  	}
	}
  for (int i=1;i<10;i++){
    if(external_orderEqual(queue[0],queue[i]) && queue[0].valid){
      removeOrder(i);
      break;
    }
    if ((queue[0].order.dir==queue[i].order.dir)||(queue[i].order.dir==NONE)){
      if ((queue[0].order.dir==UP && currentFloor<queue[i].order.floor)&&queue[i].order.floor<=queue[0].order.floor){
				queue[0].floorstop[queue[i].order.floor-1]=1;
        removeOrder(i);
        break;
      }else if ((queue[0].order.dir==DOWN && currentFloor>queue[i].order.floor)&&queue[i].order.floor>queue[0].order.floor){
				queue[0].floorstop[queue[i].order.floor-1]=1;
        removeOrder(i);
        break;
      }
    }
  }
}
