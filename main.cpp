/* On my honor, I have neither given nor received any unauthorized aid on this assignment */
#include <iostream>
#include <unordered_map>
#include <fstream>
#include <array>
#include <vector>
#include <sstream>
#include <iomanip>
#include <queue>

enum OpCodes {
    J, BEQ, BNE, BGTZ, SW, LW, BREAK, ADD, SUB, AND, OR, SRL, SRA, MUL, ADDI, ANDI, ORI
};

struct Instruction {
    std::string name;
    OpCodes opCode;
    int category;
    std::vector<int> args;

    Instruction(std::string name, OpCodes opCode, int category, std::vector<int> args);
};
struct State {
    std::array<int, 32> registers = {};
    std::unordered_map<int, int> data;
    int address = 260;
    int firstDataAddress = 0;
    Instruction *currentInstruction = nullptr;
    std::queue<Instruction*> buf1;
    std::queue<Instruction*> buf2;
    std::queue<Instruction*> buf3;
    std::queue<Instruction*> buf4;
    Instruction *buf5;
    Instruction *buf6;
    Instruction *buf7;
    Instruction *buf8;
    Instruction *buf9;
    Instruction *buf10;
    void fetch(std::unordered_map<int, Instruction> &instructions);
    void issue();
    void loadAndStore();
    void arithmetic();
    void multiply();
    void writeBack();
};

void State::fetch(std::unordered_map<int, Instruction> &instructions) {

}

void State::issue() {

}

void State::loadAndStore() {

}

void State::arithmetic() {

}

void State::multiply() {

}

void State::writeBack() {

}

Instruction::Instruction(std::string name, OpCodes opCode, int category, std::vector<int> args) :
        name(std::move(name)), opCode(opCode), category(category), args(std::move(args)) {}

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

void calculate1(Instruction &instruction, std::array<int, 32> &registers, std::unordered_map<int, int> &data, int &address) {
    int src1 = instruction.args[0];
    int src2 = instruction.args[1];
    int src3 = instruction.args[2];
    switch (instruction.opCode) {
        case J:
            address = src1;
            break;
        case BEQ:
            if (registers[src1] == registers[src2])
                address += src3 + 4;
            else
                address += 4;
            break;
        case BNE:
            if (registers[src1] != registers[src2])
                address += src3 + 4;
            else
                address += 4;
            break;
        case BGTZ:
            if (registers[src1] > 0)
                address += src2 + 4;
            else
                address += 4;
            break;
        case SW:
            data[registers[src1] + src3] = registers[src2];
            address += 4;
            break;
        case LW:
            registers[src2] = data[registers[src1] + src3];
            address += 4;
            break;
        case BREAK:
            address += 4;
        default:
            break;
    }
}

void calculate2(Instruction &instruction, std::array<int, 32> &registers) {
    // dest src1 src2
    int dest = instruction.args[0];
    int src1 = instruction.args[1];
    int src2 = instruction.args[2];
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

void calculate3(Instruction &instruction, std::array<int, 32> &registers) {
    // dest src1 imm
    int dest = instruction.args[0];
    int src1 = instruction.args[1];
    int immediate = instruction.args[2];
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
    Instruction instruction(instructionString.str(), opCodePair.second, 1, {src1, src2, src3});
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
    Instruction instruction(instructionString.str(), opCodePair.second, 2, {dest, src1, src2});
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
    Instruction instruction(instructionString.str(), opCodePair.second, 3, {dest, src1, immediate});
    instructions.emplace(address, instruction);
}

void writeRegisters(std::array<int, 32> &registers, std::ofstream &simFile) {
    simFile << "Registers\n";
    for (int i = 0; i < 4; ++i) {
        simFile << "R" << std::setw(2) << std::setfill('0') << i * 8 << ":";
        for (int j = 0; j < 8; ++j) {
            simFile << '\t' << registers[(i * 8) + j];
        }
        simFile << '\n';
    }
}
void writeData(std::unordered_map<int, int> &data, std::ofstream &simFile, int dataAddress) {
    simFile << "\nData";
    for (size_t i = 0; i < data.size(); ++i) {
        if (i % 8 == 0)
            simFile << "\n" << dataAddress << ":";
        simFile << "\t" << data[dataAddress];
        dataAddress += 4;
    }
    simFile << "\n\n";
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

void calculate(State &state) {
    switch (state.currentInstruction->category) {
        case 1:
            calculate1(*state.currentInstruction, state.registers, state.data, state.address);
            break;
        case 2:
            calculate2(*state.currentInstruction, state.registers);
            state.address += 4;
            break;
        case 3:
            calculate3(*state.currentInstruction, state.registers);
            state.address += 4;
            break;
    }
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

    while (true) {
        Instruction instruction = instructions.at(address);
        simFile << std::string(20, '-') << '\n';
        simFile << "Cycle " << cycle << ':' << "\n\n";
        state.fetch(instructions);
        state.issue();
        state.loadAndStore();
        state.arithmetic();
        state.multiply();
        state.writeBack();
        writeRegisters(state.registers, simFile);
        writeData(state.data, simFile, state.firstDataAddress);
        cycle++;
        break;
    }
    simFile.close();
    return 0;
}
