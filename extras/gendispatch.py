import yaml

def generate_instruction_table(yaml_path, output_path):
    with open(yaml_path, 'r') as f:
        data = yaml.safe_load(f)

    with open(output_path, 'w') as out:
        out.write('''#include <vector>
#include "../headers/cpu.hpp"
#include "../headers/arm.hpp"
''')

        for instr_set in data['sets']:
            var_name = instr_set['name'].lower() + "InstructionTable"
            out.write(f"std::vector<InstructionHandler> {var_name} = {{\n")

            for instr in instr_set['instructions']:
                mask = instr['mask']
                value = instr['value']
                name = instr['name']
                handler = instr['handler']
                out.write(f'    {{0x{mask:X}, 0x{value:X}, {handler}, "{name}"}},\n')

            out.write("};\n\n")

# Example usage
generate_instruction_table("extras/instructionset.yaml", "src/instruction_table.cpp")
