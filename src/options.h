#ifndef EXE_OPTIONS_H_
#define EXE_OPTIONS_H_

class Options {
public:
    Options(int argc, char* argv[]);

    enum OperationMode { REPL };
    OperationMode mode;
};

#endif

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
