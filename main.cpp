/* On my honor, I have neither given nor received any unauthorized aid on this assignment */
#include <iostream>
#include <unordered_map>
#include <fstream>
#include <array>
#include <vector>
#include <sstream>
#include <iomanip>
#include <deque>
#include <algorithm>

enum OpCodes {
    J, BEQ, BNE, BGTZ, SW, LW, BREAK, ADD, SUB, AND, OR, SRL, SRA, MUL, ADDI, ANDI, ORI
};

struct Instruction {
    std::string name;
    OpCodes opCode{};
    int category{};
    std::vector<int> args;
    int immediate{};
    std::vector<int> dependencies;
    int address{};
    bool empty = true;
    Instruction();
    Instruction(std::string name, int address, OpCodes opCode, int category, std::vector<int> args, int immediate = 0);
};

class State {
    bool foundBreak = false;
    Instruction waitInstruction;
    Instruction exeInstruction;
    std::deque<Instruction> buf1;
    std::deque<Instruction> buf2;
    std::deque<Instruction> buf3;
    std::deque<Instruction> buf4;
    Instruction buf5;
    Instruction buf6;
    Instruction buf7;
    Instruction buf8;
    Instruction buf9;
    Instruction buf10;
    void calculate1(Instruction &instruction);
    void calculate2(Instruction &instruction);
    void calculate3(Instruction &instruction);
    void calculate(Instruction &instruction);
public:
    std::array<int, 32> registers = {};
    std::unordered_map<int, int> data;
    int address = 260;
    int firstDataAddress = 0;
    void fetch(std::unordered_map<int, Instruction> &instructions, State &prevStat);
    void issue(State &prevStat, std::array<bool, 32> &registersWritten);
    void loadAndStore();
    void arithmetic(State &prevStat);
    void multiply();
    void writeBack(std::array<bool, 32> &registersWritten);
    void writeRegisters(std::ofstream &simFile);
    void writeData(std::ofstream &simFile);
    void writeState(std::ofstream &simFile);

    void cleanUp();
};

void removeInstruction(std::deque<Instruction> &buf, int address);

void State::calculate(Instruction &instruction) {
    switch (instruction.category) {
        case 1:
            calculate1(instruction);
            break;
        case 2:
            calculate2(instruction);
            break;
        case 3:
            calculate3(instruction);
            break;
    }
}
bool isBranch(Instruction &instruction) {
    OpCodes op = instruction.opCode;
    return op == J || op == BEQ || op == BNE || op == BGTZ;
}
void findDependencies(Instruction &instruction, std::deque<Instruction> &buf){
    for (auto bufInstruction : buf) {
        if (bufInstruction.args.empty() || (bufInstruction.category == 1 && bufInstruction.opCode != LW))
            continue;
        int dependency;
        if (bufInstruction.opCode == LW)
            dependency = bufInstruction.args[1];
        else
            dependency = bufInstruction.args[0];
        auto &args = instruction.args;
        if (std::any_of(args.begin(), args.end(),[dependency](auto arg){return arg == dependency;})) {
            instruction.dependencies.push_back(bufInstruction.address);
        }
    }
}

void State::fetch(std::unordered_map<int, Instruction> &instructions, State &prevStat) {
    for (int i = 0; i < 4; ++i) {
        Instruction &instruction = instructions.at(address);
        if(buf1.size() < 8 && waitInstruction.empty && exeInstruction.empty && !foundBreak) {
            findDependencies(instruction, buf1);
            if (isBranch(instruction)) {
                if (instruction.dependencies.empty()) {
                    exeInstruction = instruction;
                    exeInstruction.empty = false;
                    return;
                } else {
                    waitInstruction = instruction;
                    waitInstruction.empty = false;
                }
            } else {
                buf1.push_back(instruction);
            }
            address += 4;
        } else
            break;
    }
    if (!exeInstruction.empty) {
        calculate(exeInstruction);
        return;
    }
    if (!prevStat.waitInstruction.empty && !waitInstruction.empty && prevStat.waitInstruction.dependencies.empty()) {
        exeInstruction = waitInstruction;
        waitInstruction.empty = true;
    }
}

void State::issue(State &prevStat, std::array<bool, 32> &registersWritten) {
    auto &pBuf1 = prevStat.buf1;
    auto &prevBuf2 = prevStat.buf2;
    auto &prevBuf3 = prevStat.buf3;
    auto &prevBuf4 = prevStat.buf4;
    auto pBuf1Iter = pBuf1.begin();
    for (int i = 0; i < 6; ++i, ++pBuf1Iter) {
        if (pBuf1Iter == pBuf1.end())
            return;
        if (!pBuf1Iter->dependencies.empty())
            continue;
        auto op = (*pBuf1Iter).opCode;
        if (op != LW && op != SW) {
            // RAW && WAW
//            if (registersWritten[src1] || registersWritten[src2])
//                continue;
            if (op == MUL) {
                if (buf4.size() >= 2) {
                    continue;
                }
                buf4.push_back(*pBuf1Iter);
            } else {
                if (buf3.size() >= 2) {
                    continue;
                }
                buf3.push_back(*pBuf1Iter);
            }

            //registersWritten[dest] = true;
        } else if (op == LW) {
            auto SWLambda = [](Instruction &instruction) {return instruction.opCode == SW;};
            if (std::any_of(pBuf1.begin(), pBuf1Iter, SWLambda))
                continue;
            buf2.push_back(*pBuf1Iter);
        } else {
            auto SWLambda = [](Instruction &instruction) {return instruction.opCode == SW;};
            if (std::any_of(pBuf1.begin(), pBuf1Iter, SWLambda))
                continue;
            buf2.push_back(*pBuf1Iter);
        }
        removeInstruction(buf1, (*pBuf1Iter).address);
    }
}

void State::loadAndStore() {

}

void removeInstruction(std::deque<Instruction> &buf, int address) {
    auto findAddress = [address](Instruction &instruction) {return instruction.address == address;};
    auto removeIter = std::find_if(buf.begin(), buf.end(), findAddress);
    buf.erase(removeIter);
}

void State::arithmetic(State &prevStat) {
    auto &pBuf3 = prevStat.buf3;
    if (pBuf3.empty())
        return;
    buf6 = pBuf3.front();
    removeInstruction(buf3, buf6.address);
}

void State::multiply() {

}

void State::writeBack(std::array<bool, 32> &registersWritten) {
    int currentAddress = 0;
    auto removeDependency = [&currentAddress](int dAddress){return currentAddress == dAddress;};
    auto removeDependencies = [removeDependency](Instruction &e){
        auto removeIter = std::find_if(e.dependencies.begin(), e.dependencies.end(), removeDependency);
        if (removeIter != e.dependencies.end())
            e.dependencies.erase(removeIter);
    };
    if (!buf8.empty) {
        calculate(buf8);
    }
    if (!buf6.empty) {
        registersWritten[buf6.args[0]] = false;
        calculate(buf6);
        currentAddress = buf6.address;
        for (auto e : buf1) {
            removeDependencies(e);
        }
        if (!waitInstruction.empty)
            removeDependencies(waitInstruction);
    }
    if (!buf10.empty) {
        calculate(buf10);
    }
}

void State::cleanUp() {
    buf8.empty = true;
    buf6.empty = true;
    buf10.empty = true;
    exeInstruction.empty = true;
}

void State::calculate1(Instruction &instruction) {
    auto &args = instruction.args;
    int intermediate = instruction.immediate;
    switch (instruction.opCode) {
        case J:
            address = intermediate;
            break;
        case BEQ:
            if (registers[args[0]] == registers[args[1]])
                address += intermediate;
            break;
        case BNE:
            if (registers[args[0]] != registers[args[1]])
                address += intermediate;
            break;
        case BGTZ:
            if (registers[args[0]] > 0)
                address += intermediate;
            break;
        case SW:
            data[registers[args[0]] + intermediate] = registers[args[1]];
            break;
        case LW:
            registers[args[1]] = data[registers[args[0]] + intermediate];
            break;
        case BREAK:
            foundBreak = true;
            break;
        default:
            break;
    }
}

void State::calculate2(Instruction &instruction) {
    // dest src1 src2
    int dest = instruction.args[0];
    int src1 = instruction.args[1];
    int src2 = instruction.args.size() > 2 ? instruction.args[2] : instruction.immediate;
    switch (instruction.opCode) {
        case ADD:
            registers[dest] = registers[src1] + registers[src2];
            break;
        case SUB:
            registers[dest] = registers[src1] - registers[src2];
            break;
        case AND:
            registers[dest] = registers[src1] & registers[src2];
            break;
        case OR:
            registers[dest] = registers[src1] | registers[src2];
            break;
        case SRL:
            registers[dest] = registers[src1] << src2;
            break;
        case SRA:
            registers[dest] = registers[src1] >> src2;
            break;
        case MUL:
            registers[dest] = registers[src1] * registers[src2];
            break;
        default:
            break;
    }
}

void State::calculate3(Instruction &instruction) {
    // dest src1 imm
    int dest = instruction.args[0];
    int src1 = instruction.args[1];
    int immediate = instruction.immediate;
    switch (instruction.opCode) {
        case ADDI:
            registers[dest] = registers[src1] + immediate;
            break;
        case ANDI:
            registers[dest] = registers[src1] & immediate;
            break;
        case ORI:
            registers[dest] = registers[src1] | immediate;
            break;
        default:
            break;
    }
}

Instruction::Instruction(std::string name, int address, OpCodes opCode, int category, std::vector<int> args, int immediate) :
        name(std::move(name)), address(address), opCode(opCode), category(category), args(std::move(args)), immediate(immediate) {
    empty = false;
}

Instruction::Instruction() = default;

struct OpCodeMap {
    const std::unordered_map<std::string, std::pair<std::string, OpCodes>> cat1 = {
            {"000", std::make_pair("J", J)},
            {"001", std::make_pair("BEQ", BEQ)},
            {"010", std::make_pair("BNE", BNE)},
            {"011", std::make_pair("BGTZ", BGTZ)},
            {"100", std::make_pair("SW", SW)},
            {"101", std::make_pair("LW", LW)},
            {"110", std::make_pair("BREAK", BREAK)}
    };
    const std::unordered_map<std::string, std::pair<std::string, OpCodes>> cat2 = {
            {"000", std::make_pair("ADD", ADD)},
            {"001", std::make_pair("SUB", SUB)},
            {"010", std::make_pair("AND", AND)},
            {"011", std::make_pair("OR", OR)},
            {"100", std::make_pair("SRL", SRL)},
            {"101", std::make_pair("SRA", SRA)},
            {"110", std::make_pair("MUL", MUL)}
    };
    const std::unordered_map<std::string, std::pair<std::string, OpCodes>> cat3 = {
            {"000", std::make_pair("ADDI", ADDI)},
            {"001", std::make_pair("ANDI", ANDI)},
            {"010", std::make_pair("ORI", ORI)}
    };
};

void category1(OpCodeMap &opCodeMap, const std::string &line,
               std::unordered_map<int, Instruction> &instructions, int address) {
    std::string binaryOpCode = line.substr(3, 3);
    auto opCodePair = opCodeMap.cat1.at(binaryOpCode);
    std::stringstream instructionString;
    instructionString << opCodePair.first;
    int src1, src2, immediate = 0;
    std::vector<int> args;
    switch (opCodePair.second) {
        case J: {
            immediate = std::stoi(line.substr(6, 26), nullptr, 2);
            immediate = immediate << 2;
            instructionString << " #" << immediate;
            break;
        }
        case BNE:
        case BEQ: {
            src1 = std::stoi(line.substr(6, 5), nullptr, 2);
            src2 = std::stoi(line.substr(11, 5), nullptr, 2);
            immediate = std::stoi(line.substr(16, 16), nullptr, 2);
            immediate = immediate << 2;
            instructionString << " R" << src1 << ", R" << src2 << ", #" << immediate;
            args.insert(args.end(), {src1, src2});
            break;
        }
        case BGTZ: {
            src1 = std::stoi(line.substr(6, 5), nullptr, 2);
            immediate = std::stoi(line.substr(16, 16), nullptr, 2);
            immediate = immediate << 2;
            instructionString << " R" << src1 << ", #" << immediate;
            args.push_back(src1);
            break;
        }
        case LW:
        case SW: {
            src1 = std::stoi(line.substr(6, 5), nullptr, 2);
            src2 = std::stoi(line.substr(11, 5), nullptr, 2);
            immediate = (int16_t) std::stoi(line.substr(16, 16), nullptr, 2); // signed
            instructionString << " R" << src2 << ", " << immediate << "(R" << src1 << ")";
            args.insert(args.end(), {src1, src2});
            break;
        }
        case BREAK: {
            break;
        }
        default:
            break;
    }
    Instruction instruction(instructionString.str(), address, opCodePair.second, 1, args, immediate);
    instructions.emplace(address, instruction);

}

void category2(OpCodeMap &opCodeMap, const std::string &line,
               std::unordered_map<int, Instruction> &instructions, int address) {
    std::string binaryOpCode = line.substr(3, 3);
    auto opCodePair = opCodeMap.cat2.at(binaryOpCode);
    std::stringstream instructionString;
    instructionString << opCodePair.first << " ";
    int dest = std::stoi(line.substr(6, 5), nullptr, 2);
    int src1 = std::stoi(line.substr(11, 5), nullptr, 2);
    int src2 = std::stoi(line.substr(16, 5), nullptr, 2);
    Instruction instruction;
    switch (opCodePair.second) {
        case SRL:
        case SRA:
            instructionString << "R" << dest << ", R" << src1 << ", #" << src2;
            instruction = Instruction(instructionString.str(), address, opCodePair.second, 2, {dest, src1}, src2);
            break;
        default:
            instructionString << "R" << dest << ", R" << src1 << ", R" << src2;
            instruction = Instruction(instructionString.str(), address, opCodePair.second, 2, {dest, src1, src2});
    }
    instructions.emplace(address, instruction);
}

void category3(OpCodeMap &opCodeMap, const std::string &line,
               std::unordered_map<int, Instruction> &instructions, int address) {
    std::string binaryOpCode = line.substr(3, 3);
    auto opCodePair = opCodeMap.cat3.at(binaryOpCode);
    std::stringstream instructionString;
    instructionString << opCodePair.first << " ";
    int dest = std::stoi(line.substr(6, 5), nullptr, 2);
    int src1 = std::stoi(line.substr(11, 5), nullptr, 2);
    int immediate = std::stoi(line.substr(16, 16), nullptr, 2);
    if (opCodePair.second == ADDI) { // use two's complement
        immediate = (int16_t) immediate;
    }
    instructionString << "R" << dest << ", R" << src1 << ", #" << immediate;
    Instruction instruction(instructionString.str(), address, opCodePair.second, 3, {dest, src1}, immediate);
    instructions.emplace(address, instruction);
}
void State::writeRegisters(std::ofstream &simFile) {
    simFile << "Registers\n";
    for (int i = 0; i < 4; ++i) {
        simFile << "R" << std::setw(2) << std::setfill('0') << i * 8 << ":";
        for (int j = 0; j < 8; ++j) {
            simFile << '\t' << registers[(i * 8) + j];
        }
        simFile << '\n';
    }
}

void State::writeData(std::ofstream &simFile) {
    int currAddress = firstDataAddress;
    simFile << "\nData";
    for (size_t i = 0; i < data.size(); ++i) {
        if (i % 8 == 0)
            simFile << "\n" << currAddress << ":";
        simFile << "\t" << data[currAddress];
        currAddress += 4;
    }
    simFile << "\n\n";
}

void State::writeState(std::ofstream &simFile) {
    simFile << "IF:\n";
    simFile << "\tWaiting:" << (waitInstruction.empty ? "" : " [" + waitInstruction.name + ']') << '\n';
    simFile << "\tExecuted:" << (exeInstruction.empty ? "" : " [" + exeInstruction.name + ']') << '\n';
    auto writeBufLambda = [&simFile](int size, std::deque<Instruction> &buf){
        auto iter = buf.begin();
        for (int i = 0; i < size; ++i) {
            simFile << "\tEntry " << i << ':';
            if (iter != buf.end()) {
                simFile << " [" << (*iter).name << ']';
                iter++;
            }
            simFile << '\n';
        }
    };
    simFile << "Buf1:\n";
    writeBufLambda(8, buf1);
    simFile << "Buf2:\n";
    writeBufLambda(2, buf2);
    simFile << "Buf3:\n";
    writeBufLambda(2, buf3);
    simFile << "Buf4:\n";
    writeBufLambda(2, buf4);
    simFile << "Buf5:" << (!buf5.empty ? " [" + buf5.name + ']' : "") << '\n';
    simFile << "Buf6:" << (!buf6.empty ? " [" + buf6.name + ']' : "") << '\n';
    simFile << "Buf7:" << (!buf7.empty ? " [" + buf7.name + ']' : "") << '\n';
    simFile << "Buf8:" << (!buf8.empty ? " [" + buf8.name + ']' : "") << '\n';
    simFile << "Buf9:" << (!buf9.empty ? " [" + buf9.name + ']' : "") << '\n';
    simFile << "Buf10:" << (!buf10.empty ? " [" + buf10.name + ']' : "") << '\n';
    simFile << '\n';
}

void decodeData(State &state, std::vector<std::string> &binaryInstructions, OpCodeMap &opCodeMap, std::unordered_map<int, Instruction> &instructions) {
    bool isBreak = false;
    for (auto &binaryLine: binaryInstructions) { // loop till data section
        if (binaryLine.substr(0, 6) == "000110" && !isBreak) {
            state.firstDataAddress = state.address + 4;
            isBreak = true;
        } else if (isBreak) {
            state.data[state.address] = (int) std::stoul(binaryLine.substr(0, 32), nullptr, 2);
        }
        state.address += 4;
    }
    state.address = 260;
    for (auto &binaryLine: binaryInstructions) {
        if (state.address < state.firstDataAddress) {
            std::string category = binaryLine.substr(0, 3);
            if (category == "000")
                category1(opCodeMap, binaryLine, instructions, state.address);
            else if (category == "001")
                category2(opCodeMap, binaryLine, instructions, state.address);
            else if (category == "010")
                category3(opCodeMap, binaryLine, instructions, state.address);
        } else {
            // Todo might need to add a break
        }
        state.address += 4;
    }
    state.address = 260;
}


int main(int argc, char** argv) {
    OpCodeMap opCodeMap;
    std::ifstream inputFile(argv[1]);
    std::ofstream simFile("simulation.txt");
    State state;
    std::vector<std::string> binaryInstructions;
    std::unordered_map<int, Instruction> instructions;
    std::string line;

    while (std::getline(inputFile, line)) {
        binaryInstructions.push_back(line);
    }
    inputFile.close();

    decodeData(state, binaryInstructions, opCodeMap, instructions);

    int cycle = 1;

    State prevState = state;
    std::array<bool, 32> registersWritten = {};
    for (int i = 0; i < 20; ++i) {
        simFile << std::string(20, '-') << '\n';
        simFile << "Cycle " << cycle << ':' << "\n\n";
        state.fetch(instructions, prevState);
        state.issue(prevState, registersWritten);
        state.loadAndStore();
        state.arithmetic(prevState);
        state.multiply();
        prevState = state;
        state.writeBack(registersWritten);
        state.writeState(simFile);
        state.writeRegisters(simFile);
        state.writeData(simFile);
        state.cleanUp();
        cycle++;
    }
    simFile.close();
    return 0;
}
