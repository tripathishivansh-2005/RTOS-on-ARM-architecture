const socket = new WebSocket('ws://localhost:8080');

const state = {
    tick: 0,
    tasks: {},
    logs: []
};

const taskList = document.getElementById('task-list');
const stackList = document.getElementById('stack-list');
const logContent = document.getElementById('log-content');
const logFilters = document.getElementById('log-filters');
const tickEl = document.getElementById('system-tick');
const timeline = document.getElementById('timeline');

let currentFilter = 'All';

socket.onmessage = (event) => {
    const message = JSON.parse(event.data);

    if (message.type === 'STATE') {
        updateState(message.data);
    } else if (message.type === 'LOG') {
        addLog(message.data);
    }
};

const STACK_SIZES = {
    'LED_High': 256, 'LED_Low': 256, 'Probe': 384,
    'ISA_High': 384, 'ISA_Med': 384, 'ISA_Low': 256,
    'Sensor': 384, 'Process': 512, 'Display': 512,
    'Heartbeat': 256, 'OS_Info': 512, 'Visual': 512
};

function updateState(newState) {
    state.tick = newState.tick;
    state.tasks = newState.tasks;
    state.qA = newState.qA || 0;
    state.qB = newState.qB || 0;
    state.semaphore = newState.semaphore || 'FREE';
    state.mutex = newState.mutex || 'FREE';
    state.shared_ctr = newState.shared_ctr || 0;
    
    tickEl.innerText = state.tick;
    document.getElementById('tick-end').innerText = state.tick;
    document.getElementById('tick-start').innerText = Math.max(0, state.tick - 2000);

    renderTasks();
    renderStack();
    updateTimeline();
    updateIPC();
}

let qALatch = 0;
let qBLatch = 0;
let latchTimerA = null;
let latchTimerB = null;

function updateIPC() {
    // Shared Counter Pulse
    const ctrEl = document.getElementById('shared-ctr-val');
    if (state.shared_ctr !== parseInt(ctrEl.innerText)) {
        ctrEl.classList.remove('pulse');
        void ctrEl.offsetWidth; // Trigger reflow
        ctrEl.classList.add('pulse');
        ctrEl.innerText = state.shared_ctr;
    }

    // Queue A Latch Logic (keep dots lit for 300ms)
    if (state.qA > 0) {
        qALatch = Math.max(qALatch, state.qA);
        if (latchTimerA) clearTimeout(latchTimerA);
        latchTimerA = setTimeout(() => { qALatch = 0; updateIPCUI(); }, 300);
    }
    
    // Queue B Latch Logic
    if (state.qB > 0) {
        qBLatch = Math.max(qBLatch, state.qB);
        if (latchTimerB) clearTimeout(latchTimerB);
        latchTimerB = setTimeout(() => { qBLatch = 0; updateIPCUI(); }, 300);
    }

    updateIPCUI();

    const semEl = document.getElementById('sem-status');
    semEl.innerText = state.semaphore;
    semEl.className = (state.semaphore === 'FREE') ? 'status-ok' : 'status-warn';

    const mutexEl = document.getElementById('mutex-status');
    mutexEl.innerText = state.mutex;
    mutexEl.className = (state.mutex === 'FREE') ? 'status-ok' : 'status-err';
}

function updateIPCUI() {
    // Use the latched values for high-visibility pulses
    const qADots = document.getElementById('qA-dots').querySelectorAll('i');
    qADots.forEach((dot, i) => {
        const isActive = (i < state.qA) || (i < qALatch);
        dot.className = isActive ? 'on' : '';
    });

    const qBDots = document.getElementById('qB-dots').querySelectorAll('i');
    qBDots.forEach((dot, i) => {
        const isActive = (i < state.qB) || (i < qBLatch);
        dot.className = isActive ? 'on' : '';
    });
}

function renderTasks() {
    if (!state.tasks) return;
    taskList.innerHTML = '';
    const taskNames = Object.keys(state.tasks).sort();
    
    if (taskNames.length === 0) {
        taskList.innerHTML = '<div style="padding: 20px; color: #666; font-size: 0.9rem;">Waiting for task activity...</div>';
        return;
    }

    taskNames.forEach(name => {
        const t = state.tasks[name];
        const item = document.createElement('div');
        item.className = 'task-item';
        
        const color = getOwnerColor(t.owner);
        const isActive = (Date.now() - t.lastSeen) < 3000;
        
        item.innerHTML = `
            <span class="indicator" style="background: ${isActive ? color : '#333'}; box-shadow: ${isActive ? '0 0 8px ' + color : 'none'}"></span>
            <span class="name" style="font-weight: ${isActive ? '600' : '400'}">${name}</span>
            <span class="owner">${t.owner}</span>
            <div class="progress-bar">
                <div class="progress-fill" style="width: ${isActive ? (30 + Math.random() * 40) + '%' : '10%'}; background: ${color}"></div>
            </div>
            <span class="priority">P${getPriority(name)}</span>
        `;
        taskList.appendChild(item);
    });
}

function renderStack() {
    stackList.innerHTML = '';
    const taskNames = Object.keys(state.tasks).sort();
    
    taskNames.forEach(name => {
        const t = state.tasks[name];
        const item = document.createElement('div');
        item.className = 'stack-item';
        
        const totalSize = STACK_SIZES[name] || 256;
        // High water mark (t.stk) is the MINIMUM FREE space.
        // So free % = (t.stk / totalSize) * 100
        const freePct = Math.round((t.stk / totalSize) * 100) || 0;
        const colorClass = freePct > 30 ? 'ok' : (freePct > 15 ? 'warn' : 'err');
        
        item.innerHTML = `
            <span style="font-size: 0.85rem; width: 100px; font-weight: 500">${name}</span>
            <div class="stack-bar">
                <div class="stack-fill" style="width: ${freePct}%; background-color: var(--status-${colorClass})"></div>
            </div>
            <span style="font-size: 0.8rem; color: #fff; font-weight: 600; width: 40px; text-align: right">${freePct}%</span>
        `;
        stackList.appendChild(item);
    });
}

function addLog(line) {
    const p = document.createElement('div');
    p.className = 'log-line';
    
    // Check owner for coloring
    let owner = 'All';
    if (line.includes('[MAYANK]')) owner = 'MAYANK';
    else if (line.includes('[DEEPANSHU]')) owner = 'DEEPANSHU';
    else if (line.includes('[SHIVANSH]')) owner = 'SHIVANSH';
    else if (line.includes('[YASH]')) owner = 'YASH';
    
    p.classList.add(`log-${owner}`);
    p.innerText = line;
    
    if (currentFilter === 'All' || owner === currentFilter) {
        logContent.appendChild(p);
        logContent.scrollTop = logContent.scrollHeight;
    }
    
    // Keep log limited
    if (logContent.children.length > 100) {
        logContent.removeChild(logContent.firstChild);
    }
}

function updateTimeline() {
    const dot = document.createElement('div');
    dot.className = 'dot';
    // Randomize slightly for "activity" look
    dot.style.height = (2 + Math.random() * 8) + 'px';
    dot.style.opacity = 0.4 + Math.random() * 0.6;
    
    timeline.appendChild(dot);
    if (timeline.children.length > 500) {
        timeline.removeChild(timeline.firstChild);
    }
}

function getOwnerColor(owner) {
    switch(owner) {
        case 'MAYANK': return 'var(--accent-blue)';
        case 'DEEPANSHU': return 'var(--accent-cyan)';
        case 'SHIVANSH': return 'var(--accent-pink)';
        case 'YASH': return 'var(--accent-gold)';
        default: return '#fff';
    }
}

function getPriority(name) {
    if (name.includes('High')) return 3;
    if (name.includes('Med')) return 2;
    if (name.includes('Sensor')) return 3;
    if (name.includes('Process')) return 2;
    return 1;
}

// Log Filter Handling
logFilters.addEventListener('click', (e) => {
    if (e.target.tagName === 'BUTTON') {
        document.querySelectorAll('.tabs button').forEach(b => b.classList.remove('active'));
        e.target.classList.add('active');
        currentFilter = e.target.dataset.filter;
        logContent.innerHTML = ''; // Clear and wait for new logs
    }
});
