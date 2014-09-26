//
//  main.cpp
//  PDC_project1
//
//  Created by Prakhar Srivastava on 9/26/14.
//  Copyright (c) 2014 Rutgers. All rights reserved.
//

#include <iostream>


void init_scheduler(int sched_type);
int scheduleme(float currentTime, int tid, int remainingTime, int tprio);

void init_scheduler(int sched_type)
{

}

int scheduleme(float currentTime, int tid, int remainingTime, int tprio)
{

    std::cout << "this is inside the schedule me current time"<<currentTime<<"/n";
    std::cout << "this is inside the schedule me thread id"<<tid<<"/n";
    std::cout << "this is inside the schedule me remaining time"<<remainingTime<<"/n";
    std::cout << "this is inside the schedule me priority"<<tprio<<"/n";
    
    
    return 0;

}

