
void DebugCompareIntrons(Evaluator const & eval) override {
    const size_t instr_count = instructions.size();  // Always match instruction count

    // Step 1: Structural intron flags
    // Start with output register
    std::unordered_set<size_t> effective_registers = {0}; 
    // All instructions start off 'unmarked'
    std::vector<bool> is_effective_instruct(instructions.size(), false); 

    // Go backwards through program
    for (int i {static_cast<int>(instructions.size() - 1)}; i >= 0; --i) {
        Instruction const & temp {instructions[i]};
        // If instruction writes to an effective register, mark it as effective
        if (effective_registers.contains(temp.Ri)) { 
            is_effective_instruct[i] = true;
            // Insert operand registers into the effective set if we haven't done so already
            if (!effective_registers.contains(temp.Rj)) {
                effective_registers.insert(temp.Rj);
            }
            if (temp.Rk_type == RkType::REGISTER) {
                size_t temp_Rk {std::get<size_t>(temp.Rk)};
                if (!effective_registers.contains(temp_Rk)) {
                    effective_registers.insert(temp_Rk);
                }
            }
        }
    }

    std::vector<bool> is_structural(instructions.size(), false);
    for (size_t i {0}; i < instructions.size(); ++i) {
        is_structural[i] = !is_effective_instruct[i];
    }

    // Step 2: Semantic intron flags
    std::vector<double> input_set = eval.GetInputSet();
    size_t input_count = input_set.size();
    std::vector<std::vector<double>> before(instr_count, std::vector<double>(input_count, 0.0));
    std::vector<std::vector<double>> after(instr_count, std::vector<double>(input_count, 0.0));

    for (size_t input_idx = 0; input_idx < input_count; ++input_idx) {
        ResetRegisters();
        Input(input_set[input_idx]);
        for (size_t i = 0; i < instr_count; ++i) {
            Instruction const & instr = instructions[i];
            if (instr.Ri < registers.size()) {
                before[i][input_idx] = registers[instr.Ri];
            }
            ExecuteInstruction(instr);
            if (instr.Ri < registers.size()) {
                after[i][input_idx] = registers[instr.Ri];
            }
        }
    }

    constexpr double EPS = 1e-6;
    std::vector<bool> is_semantic(instr_count, true);
    for (size_t i = 0; i < instr_count; ++i) {
        for (size_t j = 0; j < input_count; ++j) {
            if (std::abs(before[i][j] - after[i][j]) > EPS) {
                is_semantic[i] = false;
                break;
            }
        }
    }

    // Step 3: Print mismatches
    std::cout << "Instructions marked as STRUCTURAL introns but NOT SEMANTIC:\n";
    for (size_t i = 0; i < instr_count; ++i) {
        if (!is_structural[i] && is_semantic[i]) continue; // skip true positives

        if (!is_semantic[i] && is_structural[i]) {
            std::cout << "Instruction #" << i << " is a false positive (marked structural intron only)\n";
            std::ostringstream oss;
            oss << "r[" << instructions[i].Ri << "] = r[" << instructions[i].Rj << "] "
                << GLOBAL_OPERATORS.GetOperatorName(instructions[i].op) << " ";
            if (instructions[i].Rk_type == RkType::REGISTER && std::holds_alternative<size_t>(instructions[i].Rk)) {
                oss << "r[" << std::get<size_t>(instructions[i].Rk) << "]";
            } else if (instructions[i].Rk_type == RkType::CONSTANT && std::holds_alternative<double>(instructions[i].Rk)) {
                oss << std::get<double>(instructions[i].Rk);
            }
            std::cout << "  " << oss.str() << "\n";
        }
    }
}