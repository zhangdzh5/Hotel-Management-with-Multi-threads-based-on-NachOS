//
// Created by big on 2/28/19.
//

#ifndef NACHOS_GUEST_H
#define NACHOS_GUEST_H


#include <cstdlib>
#include <iostream>
#include <math.h>
#include <vector>

class guest {
private:
    std::size_t Id;
    std::size_t rooms_num;
    std::size_t check_in_date;
    std::size_t nights_num;
    std::size_t check_out_date;
    std::vector<size_t> rooms_Id;
public:
    size_t getId() const;

    void setId(size_t Id);

    size_t getRooms_num() const;

    void setRooms_num(size_t rooms_num);

    size_t getCheck_in_date() const;

    void setCheck_in_date(size_t check_in_date);

    size_t getNights_num() const;

    void setNights_num(size_t nights_num);

    size_t getCheck_out_date() const;

    void setCheck_out_date(size_t check_out_date);

    const std::vector<size_t> &getRooms_Id() const;

    void setRooms_Id(const std::vector<size_t> &rooms_Id);

    bool operator==(const guest &other) const;

};

int guest_check_in_compare(guest l, guest r);
int guest_check_out_compare(guest l,guest r);

#endif //NACHOS_GUEST_H
