import subprocess
import os
import json

def execute_instance(exe_path: str, agents_file_path: str, tasks_file_path: str, nAgents: int, nTasks: int):
    command = [exe_path, '--a', agents_file_path, '--t', tasks_file_path]
    #command = f"{exe_path} --a {agents_file_path} --t {tasks_file_path}"

    result = subprocess.run(command, capture_output=True, check=True)
    return result.stdout.decode("UTF-8")

def extract_dict(decoded_output: str):
    lines = decoded_output.strip().split('\n')
    header_line_n = 3

    infos = dict()

    splitter = lambda s: s.split(':\t')
    infos["stats"] = {
        splitter(l)[0]: splitter(l)[1] for l in lines[:header_line_n]
    }
    infos["agents_span"] = [
        int(l.split('\t')[1])-1 for l in lines[header_line_n+1:]
    ]
    return infos

if __name__ == "__main__":
    res = execute_instance("./out/evaluation", os.path.normpath("./a40_t130/0.agents"), os.path.normpath("./a40_t130/0.tasks"), 40, 130)
    print(extract_dict(res))
