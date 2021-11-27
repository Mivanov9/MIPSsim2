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
    OpCodes opCode;
    int category;
    std::vector<int> args;
    int address;

    Instruction(std::string name, int address, OpCodes opCode, int category, std::vector<int> args);
};

class State {
    bool foundBreak = false;
    Instruction *currentInstruction = nullptr;
    Instruction *waitInstruction = nullptr;
    Instruction *exeInstruction = nullptr;
    std::deque<Instruction*> buf1;
    std::deque<Instruction*> buf2;
    std::deque<Instruction*> buf3;
    std::deque<Instruction*> buf4;
    Instruction *buf5 = nullptr;
    Instruction *buf6 = nullptr;
    Instruction *buf7 = nullptr;
    Instruction *buf8 = nullptr;
    Instruction *buf9 = nullptr;
    Instruction *buf10 = nullptr;
    void calculate1();
    void calculate2();
    void calculate3();
    void calculate();
    static bool isBranch(Instruction &instruction);
public:
    std::array<int, 32> registers = {};
    std::unordered_map<int, int> data;
    int address = 260;
    int firstDataAddress = 0;
    void fetch(std::unordered_map<int, Instruction> &instructions);
    void issue(State &prevStat, std::array<bool, 32> &registersWritten);
    void loadAndStore();
    void arithmetic(State &prevStat);
    void multiply();
    void writeBack();
    void writeRegisters(std::ofstream &simFile);
    void writeData(std::ofstream &simFile);
    void writeState(std::ofstream &simFile);

    void cleanUp();
};

void removeInstruction(std::deque<Instruction*> &buf, int address);

void State::calculate() {
    switch (currentInstruction->category) {
        case 1:
            calculate1();
            break;
        case 2:
            calculate2();
            break;
        case 3:
            calculate3();
            break;
    }
}
bool State::isBranch(Instruction &instruction) {
    OpCodes op = instruction.opCode;
    return op == J || op == BEQ || op == BNE || op == BGTZ;
}

void State::fetch(std::unordered_map<int, Instruction> &instructions) {
    for (int i = 0; i < 4; ++i) {
        Instruction &instruction = instructions.at(address);
        if(buf1.size() < 8 && waitInstruction == nullptr && !foundBreak) {
            if (isBranch(instruction)) {
                waitInstruction = &instruction;
            } else {
                buf1.push_back(&instruction);
            }
            address += 4;
        } else
            break;
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
        auto op = (*pBuf1Iter)->opCode;
        int dest = (*pBuf1Iter)->args[0];
        int src1 = (*pBuf1Iter)->args[1];
        int src2 = (*pBuf1Iter)->args[2];
        if (op != LW && op != SW) {
            // RAW
            if (registersWritten[src1] || registersWritten[src2])
                continue;
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
            removeInstruction(buf1, (*pBuf1Iter)->address);
//            auto findAddress = [pBuf1Iter](Instruction *instruction) {return instruction->address == (*pBuf1Iter)->address;};
//            auto removeIter = std::find_if(buf1.begin(), buf1.end(), findAddress);
//            buf1.erase(removeIter);
            registersWritten[dest] = true;
            continue;
        }
        if (op == LW) {
            auto SWLambda = [](Instruction* instruction) {return instruction->opCode == SW;};
            if (std::any_of(pBuf1.begin(), pBuf1Iter, SWLambda))
                continue;
        }
    }
}

void State::loadAndStore() {

}

void removeInstruction(std::deque<Instruction*> &buf, int address) {
    auto findAddress = [address](Instruction *instruction) {return instruction->address == address;};
    auto removeIter = std::find_if(buf.begin(), buf.end(), findAddress);
    buf.erase(removeIter);
}

void State::arithmetic(State &prevStat) {
    auto &pBuf3 = prevStat.buf3;
    if (pBuf3.empty())
        return;
    buf6 = pBuf3.front();
    removeInstruction(buf3, buf6->address);
}

void State::multiply() {

}

void State::writeBack() {
    if (buf8) {
        currentInstruction = buf8;
        calculate();
    }
    if (buf6) {
        currentInstruction = buf6;
        calculate();
    }
    if (buf10) {
        currentInstruction = buf10;
        calculate();
    }
}

void State::cleanUp() {
    buf8 = nullptr;
    buf6 = nullptr;
    buf10 = nullptr;
}

void State::calculate1() {
    int src1 = currentInstruction->args[0];
    int src2 = currentInstruction->args[1];
    int src3 = currentInstruction->args[2];
    switch (currentInstruction->opCode) {
        case J:
            address = src1;
            break;
        case BEQ:
            if (registers[src1] == registers[src2])
                address += src3;
            break;
        case BNE:
            if (registers[src1] != registers[src2])
                address += src3;
            break;
        case BGTZ:
            if (registers[src1] > 0)
                address += src2;
            break;
        case SW:
            data[registers[src1] + src3] = registers[src2];
            break;
        case LW:
            registers[src2] = data[registers[src1] + src3];
            break;
        case BREAK:
            foundBreak = true;
            break;
        default:
            break;
    }
}

void State::calculate2() {
    // dest src1 src2
    int dest = currentInstruction->args[0];
    int src1 = currentInstruction->args[1];
    int src2 = currentInstruction->args[2];
    switch (currentInstruction->opCode) {
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

void State::calculate3() {
    // dest src1 imm
    int dest = currentInstruction->args[0];
    int src1 = currentInstruction->args[1];
    int immediate = currentInstruction->args[2];
    switch (currentInstruction->opCode) {
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

Instruction::Instruction(std::string name, int address, OpCodes opCode, int category, std::vector<int> args) :
        name(std::move(name)), address(address), opCode(opCode), category(category), args(std::move(args)) {}

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
    int src1 = 0, src2 = 0, src3 = 0;
    switch (opCodePair.second) {
        case J: {
            src1 = std::stoi(line.substr(6, 26), nullptr, 2);
            src1 = src1 << 2;
            instructionString << " #" << src1;
            break;
        }
        case BNE:
        case BEQ: {
            src1 = std::stoi(line.substr(6, 5), nullptr, 2);
            src2 = std::stoi(line.substr(11, 5), nullptr, 2);
            src3 = std::stoi(line.substr(16, 16), nullptr, 2);
            src3 = src3 << 2;
            instructionString << " R" << src1 << ", R" << src2 << ", #" << src3;
            break;
        }
        case BGTZ: {
            src1 = std::stoi(line.substr(6, 5), nullptr, 2);
            src2 = std::stoi(line.substr(16, 16), nullptr, 2);
            src2 = src2 << 2;
            instructionString << " R" << src1 << ", #" << src2;
            break;
        }
        case LW:
        case SW: {
            src1 = std::stoi(line.substr(6, 5), nullptr, 2);
            src2 = std::stoi(line.substr(11, 5), nullptr, 2);
            src3 = (int16_t) std::stoi(line.substr(16, 16), nullptr, 2); // signed
            instructionString << " R" << src2 << ", " << src3 << "(R" << src1 << ")";
            break;
        }
        case BREAK: {
            break;
        }
        default:
            break;
    }
    Instruction instruction(instructionString.str(), address, opCodePair.second, 1, {src1, src2, src3});
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
    switch (opCodePair.second) {
        case SRL:
        case SRA:
            instructionString << "R" << dest << ", R" << src1 << ", #" << src2;
            break;
        default:
            instructionString << "R" << dest << ", R" << src1 << ", R" << src2;
    }
    Instruction instruction(instructionString.str(), address, opCodePair.second, 2, {dest, src1, src2});
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
    Instruction instruction(instructionString.str(), address, opCodePair.second, 3, {dest, src1, immediate});
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
    simFile << "\tWaiting:" << (waitInstruction == nullptr ? "" : " [" + waitInstruction->name + ']') << '\n';
    simFile << "\tExecuted:" << (exeInstruction == nullptr ? "" : " [" + exeInstruction->name + ']') << '\n';
    auto writeBufLambda = [&simFile](int size, std::deque<Instruction*> &buf){
        auto iter = buf.begin();
        for (int i = 0; i < size; ++i) {
            simFile << "\tEntry " << i << ':';
            if (iter != buf.end()) {
                simFile << " [" << (*iter)->name << ']';
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
    simFile << "Buf5:" << (buf5 ? " [" + buf5->name + ']' : "") << '\n';
    simFile << "Buf6:" << (buf6 ? " [" + buf6->name + ']' : "") << '\n';
    simFile << "Buf7:" << (buf7 ? " [" + buf7->name + ']' : "") << '\n';
    simFile << "Buf8:" << (buf8 ? " [" + buf8->name + ']' : "") << '\n';
    simFile << "Buf9:" << (buf9 ? " [" + buf9->name + ']' : "") << '\n';
    simFile << "Buf10:" << (buf10 ? " [" + buf10->name + ']' : "") << '\n';
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
    int address = 260;
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
        Instruction instruction = instructions.at(address);
        simFile << std::string(20, '-') << '\n';
        simFile << "Cycle " << cycle << ':' << "\n\n";
        state.fetch(instructions);
        state.issue(prevState, registersWritten);
        state.loadAndStore();
        state.arithmetic(prevState);
        state.multiply();
        state.writeBack();
        state.writeState(simFile);
        state.writeRegisters(simFile);
        state.writeData(simFile);
        state.cleanUp();
        prevState = state;
        cycle++;
    }
    simFile.close();
    return 0;
}
