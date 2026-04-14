const { spawn, exec } = require('child_process');
const express = require('express');
const http = require('http');
const WebSocket = require('ws');
const path = require('path');

const app = express();
const server = http.createServer(app);
const wss = new WebSocket.Server({ server });

app.use(express.static(path.join(__dirname, 'dashboard')));

// Data parsing state
let systemState = {
    tick: 0,
    tasksRunning: 11,
    scheduler: 'RUNNING',
    tasks: {},
    qA: 0,
    qB: 0,
    semaphore: 'FREE',
    mutex: 'FREE',
    shared_ctr: 0
};

// Map log prefixes to standard task names used in frontend
const TASK_MAP = {
    'MAYANK:HIGH': 'LED_High',
    'MAYANK:LOW': 'LED_Low',
    'MAYANK:PROBE': 'Probe',
    'DEEPANSHU:HIGH': 'ISA_High',
    'DEEPANSHU:MED': 'ISA_Med',
    'DEEPANSHU:LOW': 'ISA_Low',
    'SHIVANSH:SENSOR': 'Sensor',
    'SHIVANSH:PROCESS': 'Process',
    'SHIVANSH:DISPLAY': 'Display',
    'YASH:HB': 'Heartbeat',
    'YASH:OS': 'OS_Info'
};

function broadcast(data) {
    const msg = JSON.stringify(data);
    wss.clients.forEach(client => {
        if (client.readyState === WebSocket.OPEN) {
            client.send(msg);
        }
    });
}

// Start QEMU
const qemuCmd = 'D:\\qemu\\qemu-system-arm.exe';
const qemuArgs = [
    '-machine', 'mps2-an385',
    '-cpu', 'cortex-m3',
    '-kernel', 'build/rtos_demo.elf',
    '-serial', 'stdio',
    '-semihosting-config', 'enable=on,target=native'
];

console.log(`[BRIDGE] Starting QEMU: ${qemuCmd} ${qemuArgs.join(' ')}`);

// Ensure QEMU is in the path for the spawned process
const env = { ...process.env };
env.PATH = `${env.PATH};C:\\Program Files\\qemu`;

const qemu = spawn(qemuCmd, qemuArgs, { env });

qemu.stdout.on('data', (data) => {
    const output = data.toString();
    console.log(output);

    const lines = output.split('\n');
    lines.forEach(line => {
        if (!line.trim()) return;

        // Broadcast raw log for the viewer
        broadcast({ type: 'LOG', data: line.trim() });

        // Regex for common fields
        const tickMatch = line.match(/tick=(\d+)/);
        const stkMatch = line.match(/stk=(\d+)/) || line.match(/stack_free=(\d+)/);
        const nameMatch = line.match(/\[(\w+)\]\s+(\w+)/); // [DEEPANSHU] HIGH
        
        const qAMatch = line.match(/qA=(\d+)/);
        const qBMatch = line.match(/qB=(\d+)/);
        const ctrMatch = line.match(/shared_ctr=(\d+)/);
        
        // Tags: SEM_GIVEN, SEM_TAKEN, MUTEX_LOCKED, MUTEX_FREE
        if (line.includes('SEM_GIVEN')) systemState.semaphore = 'PENDING';
        if (line.includes('SEM_TAKEN')) systemState.semaphore = 'FREE';
        if (line.includes('MUTEX_LOCKED')) systemState.mutex = 'LOCKED';
        if (line.includes('MUTEX_FREE')) systemState.mutex = 'FREE';

        if (qAMatch) systemState.qA = parseInt(qAMatch[1]);
        if (qBMatch) systemState.qB = parseInt(qBMatch[1]);
        if (ctrMatch) systemState.shared_ctr = parseInt(ctrMatch[1]);

        if (tickMatch) systemState.tick = parseInt(tickMatch[1]);

        if (nameMatch) {
            const owner = nameMatch[1];
            const sub = nameMatch[2];
            const mappedName = TASK_MAP[`${owner}:${sub}`];
            
            if (mappedName) {
                if (!systemState.tasks[mappedName]) {
                    systemState.tasks[mappedName] = { owner, lastSeen: Date.now(), stk: 0 };
                }
                systemState.tasks[mappedName].lastSeen = Date.now();
                if (stkMatch) systemState.tasks[mappedName].stk = parseInt(stkMatch[1]);
            }
        }

        // Broadcast State Update
        broadcast({ type: 'STATE', data: systemState });
    });
});

qemu.stderr.on('data', (data) => {
    console.error(`[QEMU ERR] ${data}`);
});

qemu.on('close', (code) => {
    console.log(`[BRIDGE] QEMU exited with code ${code}`);
    process.exit();
});

const PORT = 8080;
server.listen(PORT, () => {
    const url = `http://localhost:${PORT}`;
    console.log(`[BRIDGE] Dashboard available at ${url}`);
    exec(`start ${url}`);
});
