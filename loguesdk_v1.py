import os
import shutil
import time
import jinja2
import re
import json

from typing import Dict, Optional

from hvcc.types.compiler import CompilerResp, ExternInfo, Generator, CompilerNotif, CompilerMsg
from hvcc.interpreters.pd2hv.NotificationEnum import NotificationEnum
from hvcc.types.meta import Meta

def set_min_value(dic, key, value):
    if key not in dic:
        dic[key] = value
    else:
        dic[key] = min(value, dic[key])
    return dic[key] == value

def render_from_template(template_file, rendered_file, context):
    templates_dir = os.path.join(os.path.dirname(__file__), "templates")
    loader = jinja2.FileSystemLoader(templates_dir)
    env = jinja2.Environment(loader=loader, trim_blocks=True, lstrip_blocks=True)
    rendered = env.get_template(template_file).render(**context)
    with open(rendered_file, 'w') as f:
        f.write(rendered)

class LogueSDK_v1(Generator):
    @classmethod
    def compile(
            cls,
            c_src_dir: str,
            out_dir: str,
            externs: ExternInfo,
            patch_name: Optional[str] = None,
            patch_meta: Meta = Meta(),
            num_input_channels: int = 0,
            num_output_channels: int = 0,
            copyright: Optional[str] = None,
            verbose: Optional[bool] = False
    ) -> CompilerResp:
        begin_time = time.time()
        print("--> Invoking LogueSDK_v1")

        out_dir = os.path.join(out_dir, "logue_unit")

        try:
            # check num of channels
            if num_input_channels != 0:
                print(f"Warning: {num_input_channels} input channels(ignored)")
            if num_output_channels != 1:
                raise Exception("Logue SDK supports only monoral output.")

            # ensure that the output directory does not exist
            out_dir = os.path.abspath(out_dir)
            if os.path.exists(out_dir):
                shutil.rmtree(out_dir)

            # copy over static files
            shutil.copytree(os.path.join(os.path.dirname(__file__), "static"), out_dir)

            # copy C files
            for file in os.listdir(c_src_dir):
                src_file = os.path.join(c_src_dir, file)
                if os.path.isfile(src_file):
                    dest_file = os.path.join(out_dir, file)
                    shutil.copy2(src_file, dest_file)

            # values for rendering templates
            context = {
                'patch_name': patch_name,
                'msg_pool_size_kb': 1,     # minimum
                'input_queue_size_kb': 1,  # minimum
                'output_queue_size_kb': 0, # minimum
            }

            # list of source files
            heavy_files_c = [
                f for f in os.listdir(c_src_dir)
                if os.path.isfile(os.path.join(out_dir, f)) and f.endswith('.c')
            ]
            context['heavy_files_c'] =  " ".join(heavy_files_c)

            heavy_files_cpp = [
                f for f in os.listdir(c_src_dir)
                if os.path.isfile(os.path.join(out_dir, f)) and f.endswith('.cpp')
            ]
            context['heavy_files_cpp'] = ' '.join(heavy_files_cpp)

            # external parameters excluding oscillator parameters
            numbered_params = []
            other_params = []
            for param in externs.parameters.inParam:
                p_name, p_rcv = param
                p_attr = p_rcv.attributes
                p_range = p_attr['max'] - p_attr['min']
                if p_name in ('pitch', 'pitch_note',
                              'noteon_trig', 'noteoff_trig',
                              'slfo'):
                    context[p_name] = {}
                    context[p_name]['hash'] = p_rcv.hash
                elif p_name in ['shape', 'alt']:
                    context[p_name] = {'name' : p_name}
                    context[p_name]['range'] = p_range
                    context[p_name]['min'] = p_attr['min']
                    context[p_name]['default'] = p_attr['default']
                    context['p_'+p_name+'hash'] = p_rcv.hash
                elif p_name in ['shape_f', 'alt_f']:
                    context[p_name[:-2]] = {'name' : p_name}
                    context[p_name[:-2]]['range_f'] = p_range
                    context[p_name[:-2]]['min'] = p_attr['min']
                    context[p_name[:-2]]['default'] = p_attr['default']
                    context[p_name[:-2]]['hash'] = p_rcv.hash
                elif re.match(r'^_\d+_', p_name):
                    numbered_params.append(param)
                else:
                    other_params.append(param)
            
            # oscillator parameters
            osc_params = [None] * 6

            # process parameter names
            for param in numbered_params:
                p_name, p_rcv = param
                match = re.match(r"^_(\d+)_(.+)$", p_name)
                param_num = int(match.group(1)) - 1
                if not (0 <= param_num <= 5):
                    raise IndexError(f"Index {param_num} is out of range.")
                p_display = match.group(2)
                if osc_params[param_num] is not None:
                    print(f'Warning: parameter {param_num} is duplicated ({osc_params[param_num]}, {p_name})')
                else:
                    osc_params[param_num] = param

            for param in other_params:
                p_name, p_rcv = param
                for i, value in enumerate(osc_params):
                    if value is None:
                        osc_params[i] = param
                        break
                else:
                    print(f"Warning: too many parameters")

            # store parameter attributes into a dcitionary (context)
            context['param'] = {}
            for i in range(6):
                if osc_params[i] is None:
                    continue
                # prefix (parameter number)
                p_key = f'param_id{i+1}'
                p_name, p_rcv = osc_params[i]
                p_attr = p_rcv.attributes
                context['param'][p_key] = {'name' : p_name}
                match = re.match(r'^_\d+_(.+)$', p_rcv.display)
                if match:
                    p_display = match.group(1)
                else:
                    p_display = p_rcv.display
                # postfix (floating-point)
                if p_name.endswith("_f"):
                    p_display = p_display[:-2]
                    p_max = p_attr['max']
                    p_min = p_attr['min']
                    p_range = p_max - p_min
                    if p_min < 0:
                        if p_max > 0:
                            p_param_max = int(-100 * max(-1, p_max / p_min))
                            p_param_min = int(100 * max(-1, p_min / p_max))
                        else:
                            # logue SDK can't set max value to less than 0
                            p_param_max = 0
                            p_param_min = -100
                    else:
                        p_param_max = 100
                        p_param_min = 0

                    if set_min_value(context['param'][p_key], 'range_f', p_range):
                        context['param'][p_key]['max_f'] = p_max
                        context['param'][p_key]['min_f'] = p_min
                        context['param'][p_key]['max'] = p_param_max
                        context['param'][p_key]['min'] = p_param_min
                else:
                    p_max = p_attr['max']
                    p_min = p_attr['min']
                    p_range = p_max - p_min
                    if set_min_value(context['param'][p_key], 'range', p_range):
                        context['param'][p_key]['max'] = max(0, min(100, int(p_max)))
                        context['param'][p_key]['min'] = max(-100, int(p_min))
                # store other key-values
                context['param'][p_key]['hash'] = p_rcv.hash
                context['param'][p_key]['name'] = p_name
                context['param'][p_key]['default'] = p_attr['default']
                context['param'][p_key]['display'] = p_display

            # find the total number of parameters
            for i in range(5, -1, -1):
                if osc_params[i] is not None:
                    num_param = i + 1
                    break
            else:
                num_param = 0
            context['num_param'] = num_param

            # store tables into a dcitionary (context)
            context['table'] = {}
            for table in externs.tables:
                t_name, t_tbl = table
                context['table'][t_name] = {'name' : t_name}
                context['table'][t_name]['hash'] = t_tbl.hash
                if t_name.endswith('_r'):
                    context['table'][t_name]['type'] = 'random'
                else:
                    context['table'][t_name]['type'] = 'none'

            # verbose
            if verbose:
                print(f"input channels:{num_input_channels}")
                print(f"output channels:{num_output_channels}")
                print(f"parameters: {externs.parameters}")
                print(f"events: {externs.events}")
                print(f"midi: {externs.midi}")
                print(f"tables: {externs.tables}")
                print(f"context: {json.dumps(context, indent=2, ensure_ascii=False)}")
            
            # estimate required heap memory
            render_from_template('Makefile.testmem',
                                 os.path.join(out_dir, "Makefile.testmem"),
                                 context)
            render_from_template('testmem.cpp',
                                 os.path.join(out_dir, "testmem.cpp"),
                                 context)
            
            # render files
            render_from_template('project.mk',
                                 os.path.join(out_dir, "project.mk"),
                                 context)
            render_from_template('logue_heavy.cpp',
                                 os.path.join(out_dir, "logue_heavy.cpp"),
                                 context)
            render_from_template('manifest.json',
                                 os.path.join(out_dir, "manifest.json"),
                                 context)

            # add definitions to HvUtils.h
            hvutils_src_path = os.path.join(c_src_dir, "HvUtils.h")
            hvutils_dst_path = os.path.join(out_dir, "HvUtils.h")
            with open(hvutils_src_path, 'r', encoding='utf-8') as f:
                src_lines = f.readlines()
    
            dst_lines = []
            for line in src_lines:
                if "// Assert" in line:
                    dst_lines.append('#include "logue_mem_hv.h"')
                    dst_lines.append("\n\n")
                elif "// Atomics" in line:
                    dst_lines.append('#include "logue_math_hv.h"')
                    dst_lines.append("\n\n")
                dst_lines.append(line)

            with open(hvutils_dst_path, 'w', encoding='utf-8') as f:
                f.writelines(dst_lines)

            # done
            end_time = time.time()

            return CompilerResp(
                stage='example_hvcc_generator',  # module name
                compile_time=end_time - begin_time,
                in_dir=c_src_dir,
                out_dir=out_dir
            )

        except Exception as e:
            return CompilerResp(
                stage="loguesdk_v1",
                notifs=CompilerNotif(
                    has_error=True,
                    exception=e,
                    warnings=[],
                    errors=[CompilerMsg(
                        enum=NotificationEnum.ERROR_EXCEPTION,
                        message=str(e)
                    )]
                ),
                in_dir=c_src_dir,
                out_dir=out_dir,
                compile_time=time.time() - begin_time
            )
