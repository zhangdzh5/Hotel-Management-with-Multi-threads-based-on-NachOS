#include "kernel.h"
#include "main.h"
#include "thread.h"
#include <iostream>
#include <vector>
#include <stdio.h>
#include <math.h>
#include <map>
#include "guest.h"
#include "scheduler.h"
#include <stdlib.h>
#include <string>
#include <memory>
#include <float.h>

void
SimpleThread(int which) {
    int num;

    for (num = 0; num < 5; num++) {
        printf("*** thread %d looped %d times\n", which, num);
        kernel->currentThread->Yield();
    }
}

/*void
ThreadTest()
{
    Thread *t = new Thread("forked thread");
    t->Fork((VoidFunctionPtr) SimpleThread, (void *) 1);
    
    SimpleThread(0);
}
*/




//----------------------------------------------------------------------
//Some global variable
//----------------------------------------------------------------------
std::size_t day = 11;
std::size_t today = 1;
vector<int> room_use(30); //check if room is available
map<int, vector<int >> room_available; //keep track of the rooms' availability
vector<double> granted_rate(day - 1); //granted rate;
vector<double> occupancy_rate(day - 1); //occupancy rate;
std::unique_ptr<SortedList<guest>> staying = std::unique_ptr<SortedList<guest>>(
        new SortedList<guest>(&guest_check_out_compare));
List<guest> checkout;
std::unique_ptr<SortedList<guest>> confirmed = std::unique_ptr<SortedList<guest>>(
        new SortedList<guest>(&guest_check_in_compare));
List<guest> discard;
map<int, Thread *> thread_map;
int guest_thread_count = 0;
double grant = 0.0;


//----------------------------------------------------------------------
//Initialize the map
//----------------------------------------------------------------------
void map_initialize() {
    for (int i = 1; i <= day; ++i) {
        room_available[i] = room_use;
    }
}

//----------------------------------------------------------------------
//Sort the list
//----------------------------------------------------------------------
/*template<typename ListType, typename SortedListType, typename PredType>
std::unique_ptr<SortedListType> rebuild_list_with_order(ListType &original, PredType &&order){
    std::unique_ptr<SortedListType> result(new SortedListType(order));
    auto iter = original.GetIterator();
    while(!iter.IsDone()){
        result->Insert(iter.Item());
        iter.Next();
    }
    return result;
}

void sort_confirm_list(){
    auto newlist = rebuild_list_with_order<SortedList<guest>, SortedList<guest>,
            int (*)(guest,guest)>(*confirmed, &guest_check_in_compare);
    *confirmed = *newlist;
}

void sort_stay_list(){
    auto newlist = rebuild_list_with_order<SortedList<guest>, SortedList<guest>,
            int (*)(guest,guest)>(*staying, &guest_check_out_compare);
    *staying = *newlist;
}
*/



//----------------------------------------------------------------------
//Print function
//----------------------------------------------------------------------
void print_request(guest req, const std::string &hint) {
    std::cout << "day " << today << std::endl;
    std::cout << hint << std::endl;
    std::cout << "The guest's information is as follows: " << std::endl;
    std::cout << "Guest id: " << req.getId() << std::endl;
    std::cout << "Rooms number: " << req.getRooms_num() << std::endl;
    if (req.getRooms_Id().empty()) {
        std::cout << "Rooms Id: No request granted." << std::endl;
    } else {
        std::cout << "Rooms Id: ";
        for (size_t i = 0; i < req.getRooms_Id().size(); ++i) {
            std::cout << req.getRooms_Id()[i] << " ";
        }
        std::cout << std::endl;
    }
    std::cout << "Check in date: " << req.getCheck_in_date() << std::endl;
    std::cout << "Check out date: " << req.getCheck_out_date() << std::endl;
    std::cout << "Nights number: " << req.getNights_num() << std::endl;
    std::cout << std::endl;
}

void print_room_num() {
    int count = 0;
    for (size_t i = 0; i < room_available[today].size(); ++i) {
        if (room_available[today][i] == 0) { count += 1; }
    }
    std::cout << "The available rooms number today is " << count << std::endl;
    std::cout << std::endl;
}

void print_summary() {
    std::cout << "This is hotel's last "<<day<<" days summary: " << std::endl;
    for (int i = 0; i < day - 1; ++i) {
        int tday = i + 1;
        std::cout << "Day " << tday << std::endl;
        std::cout << "Rooms' Occupancy Rate: " << occupancy_rate[i] << std::endl;
        std::cout << "Granted Rate: " << granted_rate[i] << std::endl;
    }
}


//----------------------------------------------------------------------
//daily summary
//----------------------------------------------------------------------
void daily_work() {
    double granted = grant / 5;
    granted_rate[today - 1] = granted;

    double counts = 0;
    for (size_t i = 0; i < room_available[today].size(); ++i) {

        if (room_available[today][i] == 1) {
            counts += 1;
        }
    }
    double count_rate = counts / 30;
    occupancy_rate[today - 1] = count_rate;
}


//----------------------------------------------------------------------
//Thread function
//----------------------------------------------------------------------
void guest_thread(size_t id) {
    guest_thread_count += 1;
    size_t guest_id = id;
    guest new_req;
    new_req.setId(guest_id);
    size_t room_num = (rand() % 5) + 1;
    new_req.setRooms_num(room_num);
    size_t night_num = (rand() % 4) + 1;
    size_t restday = day - today;
    size_t check_in = (rand() % restday) + today;
    size_t check_out = check_in + night_num;
    if (check_out > day) {
        check_out = day;
        night_num = check_out - check_in;
    }
    new_req.setCheck_in_date(check_in);
    new_req.setCheck_out_date(check_out);
    new_req.setNights_num(night_num);
    //check room available numbers
    int total_count = 0;
    for (size_t i = 0; i < room_available[check_in].size(); ++i) {
        int count = 0;
        for (int j = check_in; j < check_out; ++j) {
            if (room_available[j][i] == 0) {
                count += 1;
            }
        }
        if (count == night_num) { total_count += 1; }
    }
    //check if room is available
    if (total_count >= new_req.getRooms_num()) {
        int c_total_count = 0;
        vector<size_t> room_id;
        for (size_t i = 0; i < room_available[check_in].size(); ++i) {
            int c_count = 0;
            for (int j = check_in; j < check_out; ++j) {
                if (room_available[j][i] == 0) {
                    c_count += 1;
                }
            }
            if (c_count == night_num) {
                for (int j = check_in; j < check_out; ++j) {
                    room_available[j][i] = 1;
                }
                c_total_count += 1;
                room_id.push_back(i + 1);
            }
            if (c_total_count == new_req.getRooms_num())break;
        }
        new_req.setRooms_Id(room_id);
        /*
        if (total_count >= new_req.getRooms_num()) {
            int c_count = 0;
            vector<size_t> room_id;
            for (size_t i = 1; i <= room_use.size(); ++i) {
                if (room_use[i] == 0) {
                    room_use[i] = 1;
                    c_count += 1;
                    room_id.push_back(i);
                }
                if (c_count == new_req.getRooms_num())break;
            }
            new_req.setRooms_Id(room_id);
            */


        //count granted numbers
        grant += 1;
        print_request(new_req, "Request has been made.");
        print_room_num();
        if (new_req.getCheck_in_date() == today) {
            staying->Insert(new_req);
            print_request(new_req, "Guest has checked in.");
            thread_map[id] = kernel->currentThread;
            kernel->interrupt->SetLevel(IntOff);
            kernel->currentThread->Sleep(FALSE);
        } else {
            confirmed->Insert(new_req);
            print_request(new_req, "Request has been confirmed.");
            thread_map[id] = kernel->currentThread;
            kernel->interrupt->SetLevel(IntOff);
            kernel->currentThread->Sleep(FALSE);
        }
    } else {
        discard.Append(new_req);
        print_request(new_req, "Request has been discarded.");
        kernel->currentThread->Finish();
    }

    //after woken up
    //woken up by check in
    if (new_req.getCheck_in_date() == today) {
        staying->Insert(new_req);
        confirmed->Remove(new_req);
        print_request(new_req, "Guest has checked in.");
        kernel->interrupt->SetLevel(IntOff);
        kernel->currentThread->Sleep(FALSE);
    }

    //woken up by check out
    if (new_req.getCheck_out_date() == today) {
        checkout.Append(new_req);
        staying->Remove(new_req);
        vector<size_t> room_id;
        room_id = new_req.getRooms_Id();
        for (size_t i = 0; i < room_id.size(); ++i) {
            size_t it = room_id[i] - 1;
            for (int j = check_in; j < check_out; ++j) {
                room_available[j][it] = 0;
            }
        }
        /*
        for (size_t i = 0; i < room_id.size(); ++i) {
            size_t it = room_id[i];
            room_use[it] = 0;
        }
        */
        print_request(new_req, "Guest has checked out.");
        print_room_num();
        kernel->currentThread->Finish();
    }
    /*
    auto iter = confirm.GetIterator();
    auto iter2 = stay.GetIterator();
    while (!iter.IsDone()) {
        if (iter.Item().getId() == id) {
            stay.Append(iter.Item());
            confirm.Remove(iter.Item());
            print_request(iter.Item(), "Guest has checked in.");
            break;
        }
        iter.Next();
    }
    kernel->currentThread->Sleep(FALSE);

    while (!iter2.IsDone()) {
        if (iter2.Item().getId() == id) {
            checkout.Append(iter2.Item());
            stay.Remove(iter2.Item());
            vector<size_t> room_id;
            room_id = iter2.Item().getRooms_Id();
            for (size_t i = 0; i < room_id.size(); ++i) {
                size_t it = room_id[i];
                room_use[it] = 0;
            }

            print_request(iter2.Item(), "Guest has checked out.");
            print_room_num();
            break;
        }
        iter.Next();
    }
    kernel->currentThread->Finish();
     */

}


void concierge_thread(size_t num) {
	//set a while loop to simulate all 11 days.
    while (today <= day) {
        if (today > 1) {
			//call all qualified guest threads to check out
            if(!staying->IsEmpty()) {
                auto iter2 = staying->GetIterator();
                if (iter2.Item().getCheck_out_date() == today) {
                    while (iter2.Item().getCheck_out_date() == today) {
                        auto th_id2 = iter2.Item().getId();
                        kernel->interrupt->SetLevel(IntOff);
                        kernel->scheduler->ReadyToRun(thread_map[th_id2]);
                        iter2.Next();
                        if(iter2.IsDone()) break;
                    }
                }
            }
			//call all qualified guest threads to check in
            if(!confirmed->IsEmpty()) {
                auto iter = confirmed->GetIterator();
                if (iter.Item().getCheck_in_date() == today) {
                    while (iter.Item().getCheck_in_date() == today) {
                        auto th_id = iter.Item().getId();
                        kernel->interrupt->SetLevel(IntOff);
                        kernel->scheduler->ReadyToRun(thread_map[th_id]);
                        iter.Next();
                        if(iter.IsDone()) break;
                    }
                }
            }



            /*
            while (!iter2.IsDone()) {
                if (iter2.Item().getCheck_out_date() == today) {
                    auto th_id2 = iter2.Item().getId();
                    kernel->interrupt->SetLevel(IntOff);
                    kernel->scheduler->ReadyToRun(thread_map[th_id2]);
                }
                iter2.Next();
            }

            while (!iter.IsDone()) {
                if (iter.Item().getCheck_in_date() == today) {
                    auto th_id = iter.Item().getId();
                    kernel->interrupt->SetLevel(IntOff);
                    kernel->scheduler->ReadyToRun(thread_map[th_id]);
                }
                iter.Next();
            }
             */

        }
		//create 5 new guest threads
        if (today != day) {
            for (size_t i = 0; i < num; ++i) {
                size_t Id = 5 * (today - 1) + i + 1;
                Thread *gt = new Thread("guest thread");
                gt->Fork((VoidFunctionPtr) guest_thread, (void *) Id);
            }
        }
		//make sure all five guest threads finished or slept
        if (today != day) {
            while (1) {
                if (guest_thread_count != 5) {
                    kernel->currentThread->Yield();
                } else { break; }
            }
        } else { kernel->currentThread->Yield(); }
        daily_work();
        guest_thread_count = 0;
        grant = 0;
        today = today + 1;
    }
    print_summary();
}


void
ThreadTest() {
    map_initialize();
    Thread *ct = new Thread("Concierge thread");
    ct->Fork((VoidFunctionPtr) concierge_thread, (void *) 5);
}