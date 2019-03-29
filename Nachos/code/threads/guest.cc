//
// Created by big on 2/28/19.
//

#include "guest.h"

size_t guest::getId() const {
    return Id;
}

void guest::setId(size_t Id) {
    guest::Id = Id;
}

size_t guest::getRooms_num() const {
    return rooms_num;
}

void guest::setRooms_num(size_t rooms_num) {
    guest::rooms_num = rooms_num;
}

size_t guest::getCheck_in_date() const {
    return check_in_date;
}

void guest::setCheck_in_date(size_t check_in_date) {
    guest::check_in_date = check_in_date;
}

size_t guest::getNights_num() const {
    return nights_num;
}

void guest::setNights_num(size_t nights_num) {
    guest::nights_num = nights_num;
}

size_t guest::getCheck_out_date() const {
    return check_out_date;
}

void guest::setCheck_out_date(size_t check_out_date) {
    guest::check_out_date = check_out_date;
}

const std::vector<size_t> &guest::getRooms_Id() const {
    return rooms_Id;
}

void guest::setRooms_Id(const std::vector<size_t> &rooms_Id) {
    guest::rooms_Id = rooms_Id;
}

bool guest::operator==(const guest &other) const {
    return this->Id == other.Id;
}

int guest_check_in_compare(guest l, guest r) {
    if (l.getCheck_in_date() < r.getCheck_in_date()) {
        return -1;
    }
    if (l.getCheck_in_date() > r.getCheck_in_date()) {
        return 1;
    } else {
        return 0;
    }
}

int guest_check_out_compare(guest l, guest r) {
    if (l.getCheck_out_date() < r.getCheck_out_date()) {
        return -1;
    }
    if (l.getCheck_out_date() > r.getCheck_out_date()) {
        return 1;
    } else {
        return 0;
    }
}