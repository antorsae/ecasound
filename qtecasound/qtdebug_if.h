#ifndef INCLUDED_QTDEBUG_IF_H
#define INCLUDED_QTDEBUG_IF_H

#include <string>
#include <iostream.h>

#include <kvutils/command_queue.h>

#include <eca-debug.h>

class QTDEBUG_IF : public ECA_DEBUG {
    
public:

    void flush(void) { }

    QTDEBUG_IF(void);
    ~QTDEBUG_IF(void);

    void control_flow(const string& part);
    void msg(int level, const string& info);
};

extern QTDEBUG_IF qtdebug_if;
extern COMMAND_QUEUE qtdebug_queue;

#endif
