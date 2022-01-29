let emulator = 0;

const showError = (text) => {
    document.getElementById('err').innerText = text;
};

const appendError = (text) => {
    document.getElementById('err').innerText += text + '\n';
};

const showStatus = (text) => {
    document.getElementById('status').innerText = text;
};

const updateEmulatorStatus = () => {
    if (emulator === 0) {
        return;
    }

    for (let i = 0; i < 8; i++) {
        const val = Module.ccall('get_register_value', 'number', ['number', 'number'],
                                 [emulator, i]);
        const el = document.getElementById('reg' + i);
        el.parentNode.classList.remove('blinking');
        if (parseInt(el.value) != val) {
            el.parentNode.classList.add('blinking');
        }
        el.value = '0x' + val.toString(16);
    }
    const memOffset = Module.ccall('get_memory', 'number', ['number'], [emulator]);
    for (let i = memStart; i < memEnd; ++i) {
        const el = document.getElementById('mem' + i);
        el.parentNode.classList.remove('blinking');
        if (parseInt(el.value) != Module.HEAPU8[memOffset + i]) {
            el.parentNode.classList.add('blinking');
        }
        el.value = '0x' + Module.HEAPU8[memOffset + i].toString(16);
    }
};

let memStart = 0;
let memEnd = 0;

const createMemTable = () => {
    let start = parseInt(document.getElementById('mem_start').value);
    let end = parseInt(document.getElementById('mem_end').value);
    if (isNaN(start) || isNaN(end) || (end < start)) {
        return;
    }
    start &= ~0x7;
    if ((end & 0x7) != 0) {
        end += 8 - end & 0x7;
    }

    document.getElementById('mem_start').value = '0x' + start.toString(16);
    document.getElementById('mem_end').value = '0x' + end.toString(16);

    if (start == memStart && end == memEnd) {
        return;
    }

    memStart = start;
    memEnd = end;

    const tableBody = document.getElementById('mem_table_body');
    tableBody.innerHTML = '';

    let tr;
    for (let i = start; i < end; ++i) {
        if (i % 8 == 0) {
            tr = document.createElement('tr');
            const td = document.createElement('td');
            td.innerText = i.toString(16) + '-' + (i + 7).toString(16);
            tr.appendChild(td);
        }
        const td = document.createElement('td');
        const input = document.createElement('input');
        input.type = 'text';
        input.size = 3;
        input.id = 'mem' + i;
        td.appendChild(input);
        tr.appendChild(td);

        if (i % 8 == 7) {
            tableBody.appendChild(tr);
        }
    }
};

const blinkCurrentLine = (addr) => {
    clearBlinkLine();
    const el = document.getElementById('prog' + addr);
    el.classList.add('blinking')
    el.scrollIntoView({
        behavior: 'smooth',
        block: 'nearest',
    });
};

const clearBlinkLine = () => {
    const progLines = document.getElementById('trace_body').children;
    for (let i = 0; i < progLines.length; i++) {
        progLines[i].classList.remove('blinking', 'break');
    }
};

const createTraceTable = () => {
    if (emulator === 0) {
        return;
    }

    const progPtr = Module.ccall('dump_program', 'number', ['number'],
                                 [emulator]);
    const prog = getStringFromHeap(progPtr);
    Module._free(progPtr);

    const traceBody = document.getElementById('trace_body');
    traceBody.innerHTML = '';
    let addr = 0;
    prog.split('\n')
        .forEach(e => {
            if (e.length === 0) return;
            const tr = document.createElement('tr');
            tr.id = 'prog' + addr;
            const td1 = document.createElement('td');
            const checkBox = document.createElement('input');
            checkBox.type = 'checkbox';
            checkBox.id = 'break' + addr;
            checkBox.addEventListener('change', e => {
                const addr = parseInt(e.target.id.replace('break', ''));
                if (isNaN(addr)) {
                    return;
                }
                if (e.target.checked) {
                    Module.ccall('set_breakpoint', 'number', ['number', 'number'],
                                 [emulator, addr]);
                } else {
                    Module.ccall('remove_breakpoint', 'number', ['number', 'number'],
                                 [emulator, addr]);
                }
            });
            td1.appendChild(checkBox);
            tr.appendChild(td1);
            const td2 = document.createElement('td');
            td2.innerText = e;
            tr.appendChild(td2);
            traceBody.appendChild(tr);
            addr += 2;
        });
};

const putStringToHeap = (str) => {
    const encoder = new TextEncoder();
    const data = encoder.encode(str);
    const offset = Module._malloc(data.length);
    for (let i = 0; i < data.length; i++) {
        Module.HEAPU8[i + offset] = data[i];
    }
    return [offset, data.length];
};

const getStringFromHeap = (offset) => {
    let len;
    for (len = 0; Module.HEAPU8[offset + len] !== 0; len++) {}
    const data = Module.HEAPU8.subarray(offset, offset + len);
    const decoder = new TextDecoder();
    return decoder.decode(data);
};

let breaked = false;

const clock = () => {
    if (emulator === 0) {
        showError('Program not loaded');
        return;
    }

    showError('');

    const addr = Module.ccall('next_clock', 'number', ['number'], [emulator]);
    const breakAddr = Module.ccall('get_hit_breakpoint', 'number', [], []);
    if (breakAddr >= 0) {
        blinkCurrentLine(breakAddr);
        document.getElementById('break' + breakAddr).parentNode.parentNode
            .classList.add('break');
        breaked = true;
        document.getElementById('clock').value = 'Next clock';
    } else {
        blinkCurrentLine(addr);
        document.getElementById('exec_addr').value = '0x' + addr.toString(16);
    }

    updateEmulatorStatus();
};

const reverseClock = () => {
    if (emulator === 0) {
        showError('Program not loaded');
        return;
    }

    showError('');

    const addr = Module.ccall('reverse_next_clock', 'number', ['number'],
                              [emulator]);
    if (addr < 0) {
        clearBlinkLine();
    } else {
        blinkCurrentLine(addr);
        document.getElementById('exec_addr').value = '0x' + addr.toString(16);
    }
    updateEmulatorStatus();
};

addEventListener('load', () => {
    document.getElementById('prog_init')
        .addEventListener('click', () => {
            if (emulator !== 0) {
                Module.ccall('destroy_emulator', 'number', ['number'], [emulator])
                emulator = 0;
            }

            showError('');

            const memdata = putStringToHeap(document.getElementById('memfile').value);
            const progdata = putStringToHeap(document.getElementById('prog').value);

            emulator = Module.ccall('init_emulator', 'number',
                                    ['number', 'number', 'number', 'numer'],
                                    [memdata[0], memdata[1], progdata[0], progdata[1]]);

            Module._free(memdata[0]);
            Module._free(progdata[0]);

            createTraceTable();

            showStatus('Ready');
            updateEmulatorStatus();
        });
    document.getElementById('set_range')
        .addEventListener('click', () => {
            createMemTable();
            updateEmulatorStatus();
        });
    document.getElementById('clock')
        .addEventListener('click', e => {
            breaked = false;
            e.target.value = 'Next clock (→)';
            clock();
        });
    document.getElementById('reverse_clock')
        .addEventListener('click', () => {
            breaked = false;
            reverseClock();
        });
    document.getElementById('copy_prog_mem')
        .addEventListener('click', () => {
            if (emulator === 0) {
                showError('Program not loaded');
                return;
            }
            const progPtr = Module.ccall('dump_program', 'number', ['number'],
                                         [emulator]);
            const prog = getStringFromHeap(progPtr);
            Module._free(progPtr);
            const tmp = document.createElement('textarea');
            tmp.value = prog;
            document.body.appendChild(tmp);
            tmp.select();
            document.execCommand('copy');
            document.body.removeChild(tmp);
        });
    document.getElementById('download_mem_c')
        .addEventListener('click', () => {
            if (emulator === 0) {
                showError('Program not loaded');
                return;
            }
            const memPtr = Module.ccall('serialize_mem', 'number', ['number'],
                                        [emulator]);
            const mem = getStringFromHeap(memPtr);
            Module._free(memPtr);
            const blob = new Blob([mem], {type: "text/plain"});
            const url = URL.createObjectURL(blob);
            const link = document.createElement("a");
            document.body.appendChild(link);
            link.download = 'mem.inc';
            link.href = url;
            link.click();
            document.body.removeChild(link);
            URL.revokeObjectURL(url);
        });
    document.getElementById('apply_reg')
        .addEventListener('click', () => {
            if (emulator === 0) {
                return;
            }

            for (let i = 0; i < 8; i++) {
                const val = parseInt(document.getElementById('reg' + i).value);
                if (isNaN(val)) {
                    continue;
                }

                Module.ccall('set_register_value', 'number',
                             ['number', 'number', 'number'],
                             [emulator, i, val]);
            }
            updateEmulatorStatus();
        });
    document.getElementById('apply_mem')
        .addEventListener('click', () => {
            if (emulator === 0) {
                return;
            }

            const dataTmp = Module._malloc(memEnd - memStart);
            for (let i = 0; i < memEnd - memStart; i++) {
                const val = parseInt(document.getElementById('mem' + (i + memStart)).value);
                if (isNaN(val)) {
                    continue;
                }

                Module.HEAPU8[dataTmp + i] = val;
            }

            Module.ccall('set_mem_value', 'number',
                         ['number', 'number', 'number', 'number'],
                         [emulator, dataTmp, memEnd - memStart, memStart]);
            Module._free(dataTmp);
            updateEmulatorStatus();
        });

    createMemTable();
});

document.body.addEventListener('keydown', e => {
    if (e.target.nodeName === 'TEXTAREA' ||
        (e.target.nodeName === 'INPUT' && e.target.type === 'text')) {
        return;
    }
    if (e.key === 'ArrowRight') {
        if (breaked) {
            return;
        }

        clock();
    } else if (e.key === 'ArrowLeft') {
        breaked = false;
        reverseClock();
    }
});
