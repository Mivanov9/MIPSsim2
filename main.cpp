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
#include <set>

enum OpCodes {
    J, BEQ, BNE, BGTZ, SW, LW, BREAK, ADD, SUB, AND, OR, SRL, SRA, MUL, ADDI, ANDI, ORI
};

struct Instruction {
    std::string name;
    OpCodes opCode{};
    int category{};
    std::vector<int> args;
    int immediate{};
    std::set<int> dependencies;
    int address{};
    bool empty = true;
    int result = 0;
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
    int calculate1(Instruction &instruction);
    int calculate2(Instruction &instruction);
    int calculate3(Instruction &instruction);
    int calculate(Instruction &instruction);
public:
    std::array<int, 32> registers = {};
    std::unordered_map<int, int> data;
    int address = 260;
    int firstDataAddress = 0;
    void fetch(std::unordered_map<int, Instruction> &instructions, State &prevStat);
    void issue(State &prevStat, std::array<bool, 32> &registersWritten);
    void loadAndStore(State &prevStat);
    void arithmetic(State &prevStat);
    void multiply(State &prevStat);
    void writeBack(State &prevStat);
    void writeRegisters(std::ofstream &simFile);
    void writeData(std::ofstream &simFile);
    void writeState(std::ofstream &simFile);

    void cleanUp(State &prevStat);

    void removeDependencies(int currentAddress);

    void findDependencies(Instruction &instruction);

    static void findDependencies(std::deque<Instruction> &buf, Instruction &instruction);

    static void findDependencies(Instruction &buf, Instruction &instruction);
};

void removeInstruction(std::deque<Instruction> &buf, int address);

int State::calculate(Instruction &instruction) {
    switch (instruction.category) {
        case 1:
            return calculate1(instruction);
        case 2:
            return calculate2(instruction);
        case 3:
            return calculate3(instruction);
    }
    return 0;
}
bool isBranch(Instruction &instruction) {
    OpCodes op = instruction.opCode;
    return op == J || op == BEQ || op == BNE || op == BGTZ;
}
void State::findDependencies(std::deque<Instruction> &buf, Instruction &instruction) {
    for (auto bufInstruction: buf) {
        if (bufInstruction.args.empty())
            continue;
        int dependency;
        if (bufInstruction.category != 1 || bufInstruction.opCode == LW) {
            if (bufInstruction.opCode == LW)
                dependency = bufInstruction.args[1];
            else
                dependency = bufInstruction.args[0];
            auto &args = instruction.args;
            if (std::any_of(args.begin(), args.end(), [dependency](auto arg) { return arg == dependency; })) {
                instruction.dependencies.insert(bufInstruction.address);
            }
        }
    }
}
void State::findDependencies(Instruction &bufInstruction, Instruction &instruction) {
    if (bufInstruction.args.empty() || bufInstruction.empty)
        return;
    int dependency;
    if (bufInstruction.category != 1 || bufInstruction.opCode == LW) {
        if (bufInstruction.opCode == LW)
            dependency = bufInstruction.args[1];
        else
            dependency = bufInstruction.args[0];
        auto &args = instruction.args;
        if (std::any_of(args.begin(), args.end(), [dependency](auto arg) { return arg == dependency; })) {
            instruction.dependencies.insert(bufInstruction.address);
        }
    }
}
void State::findDependencies(Instruction &instruction){
    findDependencies(buf1, instruction);
    findDependencies(buf2, instruction);
    findDependencies(buf3, instruction);
    findDependencies(buf4, instruction);
    findDependencies(buf5, instruction);
    findDependencies(buf6, instruction);
    findDependencies(buf7, instruction);
    findDependencies(buf8, instruction);
    findDependencies(buf9, instruction);
    findDependencies(buf10, instruction);

//    for (auto bufInstruction : buf1) {
//        if (bufInstruction.args.empty())
//            continue;
//        int dependency;
//        if (bufInstruction.category != 1 || bufInstruction.opCode == LW) {
//            if (bufInstruction.opCode == LW)
//                dependency = bufInstruction.args[1];
//            else
//                dependency = bufInstruction.args[0];
//            auto &args = instruction.args;
//            if (std::any_of(args.begin(), args.end(), [dependency](auto arg) { return arg == dependency; })) {
//                instruction.dependencies.insert(bufInstruction.address);
//            }
//        }
//        if (instruction.opCode == J)
//            continue;
//        if (instruction.opCode == LW)
//            dependency = instruction.args[1];
//        else
//            dependency = instruction.args[0];
//        auto &bufArgs = bufInstruction.args;
//        if (std::any_of(bufArgs.begin(), bufArgs.end(),[dependency](auto arg){return arg == dependency;})) {
//            instruction.dependencies.insert(bufInstruction.address);
//        }

}

bool findWHazards(const std::deque<Instruction>::iterator& iter, std::deque<Instruction> &buf){
    auto &instruction = *iter;
    for (auto start = buf.begin(); start < iter; start++) {
        auto bufInstruction = *start;
        if (instruction.address == bufInstruction.address)
            continue;
        int dependency;
        if (instruction.opCode == J)
            continue;
        if (instruction.opCode == LW)
            dependency = instruction.args[1];
        else
            dependency = instruction.args[0];
        auto &bufArgs = bufInstruction.args;
        return std::any_of(bufArgs.begin(), bufArgs.end(),[dependency](auto arg){return arg == dependency;});
    }
    return false;
}

void State::fetch(std::unordered_map<int, Instruction> &instructions, State &prevStat) {
    for (int i = 0; i < 4; ++i) {
        Instruction instruction = instructions.at(address);
        if(buf1.size() < 8 && waitInstruction.empty && exeInstruction.empty && !foundBreak) {
            findDependencies(instruction);
            if (isBranch(instruction)) {
                if (instruction.dependencies.empty()) {
                    exeInstruction = instruction;
                    exeInstruction.empty = false;
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
        calculate(exeInstruction);
        waitInstruction.empty = true;
    }
}

void State::issue(State &prevStat, std::array<bool, 32> &registersWritten) {
    auto &pBuf1 = prevStat.buf1;
    auto pBuf1Iter = pBuf1.begin();
    for (int i = 0; i < 6; ++i, ++pBuf1Iter) {
        if (pBuf1Iter == pBuf1.end())
            return;
        if (!pBuf1Iter->dependencies.empty())
            continue;
        if (findWHazards(pBuf1Iter, pBuf1))
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

void removeInstruction(std::deque<Instruction> &buf, int address) {
    auto findAddress = [address](Instruction &instruction) {return instruction.address == address;};
    auto removeIter = std::find_if(buf.begin(), buf.end(), findAddress);
    buf.erase(removeIter);
}

void State::loadAndStore(State &prevStat) {
    // ALU 1
    auto &pBuf2 = prevStat.buf2;
    if (!pBuf2.empty()) {
        buf5 = pBuf2.front();
        removeInstruction(buf2, buf5.address);
    }
    // MEM
    auto &pBuf5 = prevStat.buf5;
    if (!pBuf5.empty) {
        if (pBuf5.opCode == LW) {
            buf8 = pBuf5;
            buf8.result = calculate(buf8);
            std::stringstream s;
            s << buf8.result << ", R" << buf8.args[1];
            buf8.name = s.str();
        } else {
            calculate(pBuf5);
        }
    }
}

void State::arithmetic(State &prevStat) {
    // ALU 2
    auto &pBuf3 = prevStat.buf3;
    if (pBuf3.empty())
        return;
    buf6 = pBuf3.front();
    buf6.result = calculate(buf6);
    std::stringstream s;
    s << buf6.result << ", R" << buf6.args[0];
    buf6.name = s.str();
    removeInstruction(buf3, buf6.address);
}

void State::multiply(State &prevStat) {
    // MUL1
    auto &pBuf4 = prevStat.buf4;
    if (!pBuf4.empty()) {
        buf7 = pBuf4.front();
        removeInstruction(buf4, buf7.address);
    }
    // MUL2
    auto &pBuf7 = prevStat.buf7;
    if (!pBuf7.empty) {
        buf9 = pBuf7;
    }
    // MUL3
    auto &pBuf9 = prevStat.buf9;
    if (!pBuf9.empty) {
        buf10 = pBuf9;
        buf10.result = calculate(buf10);
        std::stringstream s;
        s << buf10.result << ", R" << buf10.args[0];
        buf10.name = s.str();
    }
}
void State::removeDependencies(int currentAddress) {
    auto removeDependency = [&currentAddress](int dAddress){return currentAddress == dAddress;};
    for (auto &e : buf1) {
        auto removeIter = std::find_if(e.dependencies.begin(), e.dependencies.end(), removeDependency);
        if (removeIter != e.dependencies.end())
            e.dependencies.erase(removeIter);
    }
    auto removeIter = std::find_if(waitInstruction.dependencies.begin(), waitInstruction.dependencies.end(), removeDependency);
    if (removeIter != waitInstruction.dependencies.end())
        waitInstruction.dependencies.erase(removeIter);
}

void State::writeBack(State &prevStat) {
    if (!prevStat.buf8.empty) {
        registers[prevStat.buf8.args[1]] = prevStat.buf8.result;
        removeDependencies(prevStat.buf8.address);
    }
    if (!prevStat.buf6.empty) {
        registers[prevStat.buf6.args[0]] = prevStat.buf6.result;
        removeDependencies(prevStat.buf6.address);
    }
    if (!prevStat.buf10.empty) {
        registers[prevStat.buf10.args[0]] = prevStat.buf10.result;
        removeDependencies(prevStat.buf10.address);
    }
}

void State::cleanUp(State &prevStat) {
    buf8.empty = true;
    buf5.empty = true;
    buf6.empty = true;
    buf10.empty = true;
    buf7.empty = true;
    buf9.empty = true;
    exeInstruction.empty = true;
}

int State::calculate1(Instruction &instruction) {
    auto &args = instruction.args;
    int intermediate = instruction.immediate;
    switch (instruction.opCode) {
        case J:
            address = intermediate;
            return 0;
        case BEQ:
            if (registers[args[0]] == registers[args[1]])
                address += intermediate;
            return 0;
        case BNE:
            if (registers[args[0]] != registers[args[1]])
                address += intermediate;
            return 0;
        case BGTZ:
            if (registers[args[0]] > 0)
                address += intermediate;
            return 0;
        case SW:
            data[registers[args[0]] + intermediate] = registers[args[1]];
            return 0;
        case LW:
            //registers[args[1]] = data[registers[args[0]] + intermediate];
            return data[registers[args[0]] + intermediate];
        case BREAK:
            foundBreak = true;
            return 0;
        default:
            return 0;
    }
}

int State::calculate2(Instruction &instruction) {
    // dest src1 src2
    int dest = instruction.args[0];
    int src1 = instruction.args[1];
    int src2 = instruction.args.size() > 2 ? instruction.args[2] : instruction.immediate;
    switch (instruction.opCode) {
        case ADD:
            return registers[src1] + registers[src2];
        case SUB:
            return registers[src1] - registers[src2];
        case AND:
            return registers[src1] & registers[src2];
        case OR:
            return registers[src1] | registers[src2];
        case SRL:
            return registers[src1] << src2;
        case SRA:
            return registers[src1] >> src2;
        case MUL:
            return registers[src1] * registers[src2];
        default:
            return 0;
    }
}

int State::calculate3(Instruction &instruction) {
    // dest src1 imm
    int dest = instruction.args[0];
    int src1 = instruction.args[1];
    int immediate = instruction.immediate;
    switch (instruction.opCode) {
        case ADDI:
            return registers[src1] + immediate;
        case ANDI:
            return registers[src1] & immediate;
        case ORI:
            return registers[src1] | immediate;
        default:
            return 0;
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
    for (int i = 0; i < 62; ++i) {
        simFile << std::string(20, '-') << '\n';
        simFile << "Cycle " << cycle << ':' << "\n\n";
        state.fetch(instructions, prevState);
        state.issue(prevState, registersWritten);
        state.loadAndStore(prevState);
        state.arithmetic(prevState);
        state.multiply(prevState);
        prevState = state;
        state.writeState(simFile);
        state.writeRegisters(simFile);
        state.writeData(simFile);
        state.writeBack(prevState);
        state.cleanUp(prevState);
        cycle++;
    }
    simFile.close();
    return 0;
}
