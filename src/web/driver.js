const editorData = {
    'editor_prog': {
        model: null,
        state: null,
    },
    'editor_mem': {
        model: null,
        state: null,
    },
};

const defaultContents = {
    'editor_prog': [
        'exasm',
        `lli r0, 0x30 # first word
lli r1, 0x40 # last word

# If you put "@foo" at the beginning of
# line, you can refer to the position from
# operands of jump/branch instructions.
@loop sw r2, (r0)
addi r0, 2
mov r3, r1
sub r3, r0
# "@loop" is replaced with actual address
# of "sw" instruction at line 7.
bnez r3, @loop
nop

# You can use "@foo" address syntax to
# jump to any position on the source code.
@dyn_stop j @dyn_stop
nop`
    ],
    'editor_mem': [
        'memfile',
        `@30 00000001 00000010
@32 00000011 00000100
@34 00000101 00000110
@36 00000111 00001000
@38 00001001 00001010
@3a 00001011 00001100
@3c 00001101 00001110
@3e 00001111 00010000`
    ],
};

let emulator = 0;

const showError = (text) => {
    document.getElementById('err').innerText = text;
};

const appendError = (text) => {
    document.getElementById('err').innerText += text + '\n';
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

    const clockCount = Module.ccall('get_estimated_clock', 'number', ['number'], [emulator]);
    document.getElementById('clock_count').innerText = clockCount;
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
            const td = document.createElement('td');
            const checkBox = document.createElement('input');
            checkBox.type = 'checkbox';
            checkBox.id = 'break' + addr;
            checkBox.classList.add('breakpoint');
            checkBox.addEventListener('change', e => {
                const addr = parseInt(e.target.id.replace('break', ''));
                if (isNaN(addr)) {
                    return;
                }
                if (e.target.checked) {
                    states.breakpoints = states.breakpoints.concat({pc: addr}).sort((a, b) => {
                        if (typeof a.pc === 'undefined' || typeof b.pc === 'undefined') return 0;
                        return a.pc < b.pc ? -1 : 1;
                    });
                    Module.ccall('set_breakpoint', 'number', ['number', 'number'],
                                 [emulator, addr]);
                } else {
                    states.breakpoints = states.breakpoints.filter(e => typeof e.pc === 'undefined' || e.pc !== addr);
                    Module.ccall('remove_breakpoint', 'number', ['number', 'number'],
                                 [emulator, addr]);
                }
            });
            td.appendChild(checkBox);
            const label = document.createElement('label');
            label.setAttribute('for', 'break' + addr);
            td.appendChild(label);
            const inst = document.createElement('span');
            inst.innerText = e;
            td.appendChild(inst);
            tr.appendChild(td);
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

const states = new Proxy(
    {
        continueInterrupted: true,
        breaked: false,
        breakpoints: [],
    },
    {
        set: (obj, prop, value) => {
            if (prop in obj) {
                if (prop === 'continueInterrupted') {
                    if (value) {
                        document.getElementById('continue').innerHTML = '<span class="material-icons">' +
                            'rotate_right</span><br>Continue';
                    } else {
                        document.getElementById('continue').innerHTML = '<span class="material-icons">' +
                            'stop</span><br>Interrupt';
                    }
                } else if (prop === 'breaked') {
                    document.getElementById('clock').title = value ? 'Next Clock' : 'Next Clock (→)';
                } else if (prop == 'breakpoints') {
                    const table = document.getElementById('breakpoint_table_body');
                    table.innerHTML = '';
                    value.forEach(e => {
                        const tr = document.createElement('tr');
                        const td = document.createElement('td');

                        const deleteButton = document.createElement('span');
                        deleteButton.innerText = 'delete_outline';
                        deleteButton.addEventListener('click', () => {
                            if (typeof e.pc !== 'undefined') {
                                document.getElementById('break' + e.pc).click();
                            }
                        });
                        deleteButton.classList.add('material-icons', 'button', 'delete');
                        td.appendChild(deleteButton);
                        tr.appendChild(td);

                        const condition = document.createElement('span');
                        if (typeof e.pc !== 'undefined') {
                            condition.innerText = `Fetch 0x${e.pc.toString(16)}`;
                        }
                        td.appendChild(condition);

                        table.appendChild(tr);
                    });
                }

                obj[prop] = value;
                return true;
            }
            return false;
        },
    }
);

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
        states.breaked = true;
    } else {
        blinkCurrentLine(addr);
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
    }
    updateEmulatorStatus();
};

const doContinue = () => {
    if (emulator === 0) {
        showError('Program not loaded');
        return;
    }

    for (let i = 0; i < 10; i++) {
        clock();
        if (states.breaked) {
            states.continueInterrupted = true;
            return;
        } else if (states.continueInterrupted) {
            return;
        }
    }
    setTimeout(doContinue);
};

let editor;

const initEditor = () => {
    monaco.languages.register({
        id: 'exasm'
    });
    monaco.languages.setMonarchTokensProvider('exasm', {
        tokenizer: {
            root: [
                [/r[0-7]/, 'predefined'],
                [/[()]/, {open: '(', close: ')', token: 'delimiter.parenthesis'}],
                [/[a-z][a-z0-9]*/, 'keyword'],
                [/@[a-zA-Z_][a-zA-Z0-9_]*/, 'identifier'],
                [/-?0x[0-9a-fA-F]+/, 'number.hex'],
                [/-?0[0-9a-fA-F]+/, 'number.oct'],
                [/-?[0-9]+/, 'number'],
                [/#.*$/, 'comment'],
                [/ \t,/, 'white'],
            ],
        },
    });
    monaco.languages.register({
        id: 'memfile'
    });
    monaco.languages.setMonarchTokensProvider('memfile', {
        tokenizer: {
            root: [
                [/^@[0-9a-fA-F]+/, 'number.hex'],
                [/[01]/, 'number'],
                [/\/\/.*$/, 'comment'],
                [/ \t,/, 'white'],
            ],
        },
    });

    let enabledTab = 'editor_prog';
    for (const tab of Object.keys(editorData)) {
        if (document.getElementById(tab).checked) {
            enabledTab = tab;
        }
        let content = defaultContents[tab][1];
        const oldContent = localStorage.getItem(tab);
        if (oldContent !== null) {
            content = oldContent;
        }

        editorData[tab].model = monaco.editor.createModel(content, defaultContents[tab][0])
    }

    editor = monaco.editor.create(document.getElementById('prog_editor'), {
        model: editorData[enabledTab].model,
        minimap: {enabled: false},
        theme: 'vs-dark',
        automaticLayout: true,
    });

    for (const tab of ['editor_prog', 'editor_mem']) {
        document.getElementById(tab)
            .addEventListener('change', e => {
                if (e.target.checked) {
                    const newTab = e.target.id;
                    const oldTab = e.target.id === 'editor_prog' ? 'editor_mem' : 'editor_prog';
                    editorData[oldTab].state = editor.saveViewState();
                    editorData[oldTab].model = editor.getModel();
                    editor.setModel(editorData[newTab].model);
                    editor.restoreViewState(editorData[newTab].state);
                    editor.focus();

                    localStorage.setItem(oldTab, editorData[oldTab].model.getValue());
                }
            });
    }
};

addEventListener('load', () => {
    document.getElementById('prog_init')
        .addEventListener('click', () => {
            states.continueInterrupted = true;
            states.breaked = false;

            localStorage.setItem('editor_prog', editorData['editor_prog'].model.getValue());
            localStorage.setItem('editor_mem', editorData['editor_mem'].model.getValue());

            setTimeout(() => {
                if (emulator !== 0) {
                    Module.ccall('destroy_emulator', 'number', ['number'], [emulator])
                    emulator = 0;
                }

                showError('');

                const memdata = putStringToHeap(editorData['editor_mem'].model.getValue());
                const progdata = putStringToHeap(editorData['editor_prog'].model.getValue());

                emulator = Module.ccall('init_emulator', 'number',
                                        ['number', 'number', 'number', 'numer'],
                                        [memdata[0], memdata[1], progdata[0], progdata[1]]);

                Module._free(memdata[0]);
                Module._free(progdata[0]);

                createTraceTable();
                blinkCurrentLine(0);

                updateEmulatorStatus();
            });
        });
    document.getElementById('set_range')
        .addEventListener('click', () => {
            createMemTable();
            updateEmulatorStatus();
        });
    document.getElementById('clock')
        .addEventListener('click', e => {
            states.continueInterrupted = true;
            states.breaked = false;
            clock();
        });
    document.getElementById('reverse_clock')
        .addEventListener('click', () => {
            states.continueInterrupted = true;
            states.breaked = false;
            reverseClock();
        });
    document.getElementById('continue')
        .addEventListener('click', e => {
            states.breaked = false;
            if (states.continueInterrupted) {
                states.continueInterrupted = false;
                doContinue();
            } else {
                states.continueInterrupted = true;
            }
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

    document.getElementById('close_notification')
        .addEventListener('click', e => {
            e.target.parentNode.animate([{opacity: 1}, {opacity: 0}], 300)
                .addEventListener('finish', () => {
                    e.target.parentNode.remove();
                });
        });

    createMemTable();
    initEditor();
});

document.body.addEventListener('keydown', e => {
    if (e.target.nodeName === 'INPUT' && e.target.type === 'text') {
        return;
    }
    if (e.key === 'ArrowRight') {
        states.continueInterrupted = true;

        if (states.breaked) {
            return;
        }

        e.preventDefault();
        clock();
    } else if (e.key === 'ArrowLeft') {
        e.preventDefault();
        states.breaked = false;
        states.continueInterrupted = true;
        reverseClock();
    }
});
