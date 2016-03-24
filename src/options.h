#ifndef OPTIONS_H_
#define OPTIONS_H_

class Options {
public:
    Options(int argc, char* argv[]);

    enum OperationMode { REPL };
    const Options::OperationMode operation_mode;

    struct ReplOptions {
        bool disassemble;
    } repl_options;
};

#endif

// vim: ts=4:sw=4:sts=4:expandtab
