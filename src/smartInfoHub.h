#ifndef SMARTINFOHUB_H_
#define SMARTINFOHUB_H_

#include <iostream>

class SmartInfoHub
{
  public:
    SmartInfoHub();
    static void smartInfo(int);
    static void setAnswer(int);
    static int getInformation();
  private:
    static int answer;
};
#endif
