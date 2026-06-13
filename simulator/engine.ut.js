// engine.ut.js
// Unifies JS and C/C++ engine logic for ESP32 HUB75
class SuperArtEngine {
    constructor(width, height, ctx) {
        this.width = width;
        this.height = height;
        this.ctx = ctx;
        this.time = 0;
        this.particles = [];
        this.grid = [];
        this.pegs = [];
        
        // --- FAST MATH LOOKUP TABLES (LUTs) ---
        this.lutSize = 4096;
        this.sinLUT = new Float32Array(this.lutSize);
        this.cosLUT = new Float32Array(this.lutSize);
        for (let i = 0; i < this.lutSize; i++) {
            let angle = (i / this.lutSize) * Math.PI * 2;
            this.sinLUT[i] = Math.sin(angle);
            this.cosLUT[i] = Math.cos(angle);
        }

        this.maxRadius = Math.ceil(Math.sqrt(width*width + height*height));

        // Base 40 Named Palettes
        const basePalettes = [
            { name: "Plasma", colors: [[255,0,0], [255,127,0], [255,255,0], [255,0,255], [255,255,255]] },
            { name: "Ice", colors: [[0,255,255], [0,136,255], [0,0,255], [255,255,255], [136,204,255]] },
            { name: "Fire", colors: [[255,0,0], [255,68,0], [255,136,0], [255,204,0], [34,0,0]] },
            { name: "Synthwave", colors: [[255,0,255], [0,255,255], [43,0,255], [255,0,85], [0,0,34]] },
            { name: "Cyberpunk", colors: [[252,238,10], [255,0,60], [0,240,255], [0,0,0], [17,17,17]] },
            { name: "Forest", colors: [[0,68,0], [0,136,0], [34,204,34], [136,255,136], [34,17,0]] },
            { name: "Ocean", colors: [[0,17,51], [0,51,102], [0,102,153], [51,153,204], [255,255,255]] },
            { name: "Sunset", colors: [[51,0,51], [102,0,51], [255,51,51], [255,153,51], [255,255,102]] },
            { name: "Galaxy", colors: [[0,0,0], [34,0,68], [68,0,136], [136,0,255], [255,255,255]] },
            { name: "Toxic", colors: [[0,0,0], [17,51,0], [51,102,0], [102,204,0], [204,255,0]] },
            { name: "Desert", colors: [[255,204,136], [221,170,221], [170,136,102], [136,68,34], [68,34,17]] },
            { name: "Candy", colors: [[255,204,204], [255,153,204], [255,102,204], [255,51,204], [255,255,255]] },
            { name: "Mono", colors: [[0,0,0], [51,51,51], [136,136,136], [204,204,204], [255,255,255]] },
            { name: "Retro", colors: [[217,91,67], [192,41,66], [84,36,55], [83,119,122], [236,208,120]] },
            { name: "Matrix", colors: [[0,0,0], [0,51,0], [0,102,0], [0,255,0], [204,255,204]] },
            { name: "Vapor", colors: [[255,113,206], [1,205,254], [5,255,161], [185,103,255], [255,251,150]] },
            { name: "Gold", colors: [[34,17,0], [102,68,0], [170,136,0], [221,204,0], [255,255,170]] },
            { name: "Blood", colors: [[17,0,0], [68,0,0], [136,0,0], [204,0,0], [255,0,0]] },
            { name: "Aurora", colors: [[0,0,0], [0,34,17], [0,102,68], [0,255,136], [170,255,204]] },
            { name: "Space", colors: [[0,0,0], [8,8,24], [16,16,48], [48,48,96], [255,255,255]] },
            { name: "Lava", colors: [[0,0,0], [34,0,0], [170,0,0], [255,68,0], [255,255,0]] },
            { name: "Electric", colors: [[0,0,34], [0,0,136], [0,68,255], [0,170,255], [255,255,255]] },
            { name: "Pastel", colors: [[255,179,186], [255,223,186], [255,255,186], [186,255,201], [186,225,255]] },
            { name: "Midnight", colors: [[10,10,42], [26,26,74], [42,42,106], [74,74,138], [170,170,170]] },
            { name: "Neon", colors: [[255,0,0], [0,255,0], [0,0,255], [255,255,0], [255,0,255]] },
            { name: "Kaotic", colors: [[255,0,255], [0,255,0], [255,0,0], [0,0,255], [255,255,0]] },
            { name: "Fade Red", colors: [[20,0,0], [80,0,0], [150,0,0], [220,0,0], [255,100,100]] },
            { name: "Fade Green", colors: [[0,20,0], [0,80,0], [0,150,0], [0,220,0], [100,255,100]] },
            { name: "Fade Blue", colors: [[0,0,20], [0,0,80], [0,0,150], [0,0,220], [100,100,255]] },
            { name: "Void", colors: [[0,0,0], [10,10,10], [25,25,25], [10,10,10], [0,0,0]] },
            { name: "Ghost", colors: [[200,200,255], [220,220,255], [240,240,255], [255,255,255], [200,200,255]] },
            { name: "Hot Pink", colors: [[50,0,20], [150,0,60], [255,0,100], [255,100,150], [255,200,220]] },
            { name: "Acid", colors: [[0,0,0], [100,255,0], [200,0,255], [0,255,200], [255,255,255]] },
            { name: "Autumn", colors: [[100,30,0], [150,60,0], [200,100,0], [255,150,0], [255,200,50]] },
            { name: "Glitch", colors: [[255,0,0], [0,255,255], [0,255,0], [255,0,255], [0,0,255]] },
            // NEW PALETTES
            { name: "LSD", colors: [[255,0,128], [128,255,0], [0,128,255], [255,255,0], [0,255,255]] },
            { name: "Flashy", colors: [[255,255,255], [255,0,0], [255,255,0], [0,255,0], [0,255,255]] },
            { name: "Psychedelic", colors: [[200,0,255], [0,255,100], [255,100,0], [0,100,255], [255,200,0]] },
            { name: "Rave", colors: [[0,255,50], [255,0,150], [50,0,255], [255,255,0], [255,0,0]] },
            { name: "Rainbow", colors: [[255,0,0], [255,127,0], [255,255,0], [0,255,0], [0,0,255]] }
        ];

        // Generate 256 colors per palette
        this.palettes = basePalettes.map(bp => {
            let colors256 = [];
            for (let i = 0; i < 256; i++) {
                let t = i / 255.0; 
                let segment = t * 4;
                let idx1 = Math.floor(segment);
                let idx2 = Math.min(4, idx1 + 1);
                let localT = segment - idx1;
                let c1 = bp.colors[idx1];
                let c2 = bp.colors[idx2];
                let r = Math.round(c1[0] + (c2[0] - c1[0]) * localT);
                let g = Math.round(c1[1] + (c2[1] - c1[1]) * localT);
                let b = Math.round(c1[2] + (c2[2] - c1[2]) * localT);
                colors256.push(`rgb(${r},${g},${b})`);
            }
            return { name: bp.name, colors: colors256 };
        });

        // 100 Light FX Styles
        this.lightFx = [];
        const blendModes = ['source-over', 'lighter', 'screen', 'overlay', 'color-dodge'];
        for (let i = 0; i < 100; i++) {
            let bMode = blendModes[i % blendModes.length];
            let glow = (i % 10) * 2; 
            let alphaMod = 0.5 + ((i % 5) * 0.1);
            this.lightFx.push({
                name: `FX-${i+1} (${bMode}, G:${glow})`,
                blendMode: bMode,
                shadowBlur: glow,
                alphaMod: alphaMod
            });
        }

        // Expanded Algorithms
        this.algorithms = [
            "0: Conway's Game of Life", 
            "1: Mandelbrot Set", 
            "2: Substrate (Tarbell)", 
            "3: Fractal Flame (Draves)",
            "4: L-Systems (Lindenmayer)",
            "5: Particle Flow Field", 
            "6: Plasma Grid", 
            "7: Neural Morph",
            "8: Plotter Generative", 
            "9: LED Pulse Volume", 
            "10: Geometric Wave", 
            "11: Squiggle Art (Snowfro)",
            "12: Divided by 16 (Molnar)",
            "13: Random War (Nake)",
            "14: Computergrafik (Nees)",
            "15: Drawing 118 (LeWitt)",
            "16: Fidenza (Hobbs)",
            "17: Ringers (Cherniak)",
            "18: Autoglyphs (Larva Labs)",
            "19: Archetype (Golid)",
            "20: Memories of Passersby I",
            "21: Unsupervised (Anadol)",
            "22: Learning to See (Akten)",
            "23: Avid Lines (Arihz)",
            "24: AI Generative Boids Swarm"
        ];

        const adj = ["Cosmic", "Neon", "Quantum", "Hyper", "Digital", "Mystic", "Dark", "Radiant", "Chaotic", "Fluid"];
        const noun = ["Dream", "Matrix", "Wave", "Pulse", "Void", "Engine", "Storm", "Core", "Realm", "Shift"];

        // 400 Named Formulas
        this.animations = [];
        for (let i = 0; i < 400; i++) {
            let seed = i + 1; 
            let algo_type = Math.floor(Math.abs(Math.sin(seed * 12.34)) * this.algorithms.length) % this.algorithms.length;
            let palette_idx = Math.floor(Math.abs(Math.cos(seed * 56.78)) * this.palettes.length) % this.palettes.length;
            let fx_idx = Math.floor(Math.abs(Math.sin(seed * 34.12)) * 100) % 100;
            let shape_idx = Math.floor(Math.abs(Math.cos(seed * 91.23)) * 100) + 1;
            let p_type = Math.floor(Math.abs(Math.sin(seed * 22.33)) * 100) + 1;
            
            let symmetry = Math.floor(Math.abs(Math.cos(seed * 44.55)) * 5); 
            let colorSpeed = parseFloat((Math.abs(Math.sin(seed * 66.77)) * 5.0 + 0.1).toFixed(2));
            let bpm = Math.floor(Math.abs(Math.cos(seed * 88.99)) * 140) + 60; 
            let playDuration = Math.floor(Math.abs(Math.sin(seed * 77.88)) * 55) + 5; 
            
            let speed = 0.5 + Math.abs(Math.sin(seed * 90.12)) * 3.0;
            let density = Math.max(10, 50 + Math.floor(Math.abs(Math.cos(seed * 34.56)) * 950)); 
            let complexity = 1.0 + Math.abs(Math.sin(seed * 78.90)) * 5.0;
            let chaos = parseFloat((Math.abs(Math.cos(seed * 11.11)) * 10.0).toFixed(3));
            
            let mathA = parseFloat((Math.abs(Math.sin(seed * 1.11)) * 1.9 + 0.1).toFixed(3));
            let mathB = parseFloat((Math.abs(Math.cos(seed * 2.22)) * 1.9 + 0.1).toFixed(3));
            let mathC = parseFloat((Math.abs(Math.sin(seed * 3.33)) * 9.9 + 0.1).toFixed(3));
            let mathD = parseFloat((Math.abs(Math.cos(seed * 4.44)) * 9.9 + 0.1).toFixed(3));
            let mathE = parseFloat((Math.abs(Math.sin(seed * 5.55)) * 9.9 + 0.1).toFixed(3));
            let mathF = parseFloat((Math.abs(Math.cos(seed * 6.66)) * 9.9 + 0.1).toFixed(3));
            
            let name = `${adj[seed % adj.length]} ${noun[(seed*7) % noun.length]} ${seed}`;
            
            this.animations.push({
                id: seed, name: name, algo: algo_type, palette: palette_idx, lightFx: fx_idx,
                shape: shape_idx, particleType: p_type, symmetry: symmetry, colorSpeed: colorSpeed,
                bpm: bpm, playDuration: playDuration, chaos: chaos, speed: parseFloat(speed.toFixed(3)),
                density: density, complexity: parseFloat(complexity.toFixed(3)), 
                trailFade: parseFloat((0.1 + Math.abs(Math.sin(seed * 1.23)) * 0.4).toFixed(3)),
                mathA: mathA, mathB: mathB, mathC: mathC, mathD: mathD, mathE: mathE, mathF: mathF
            });
        }
        
        this.currentAnim = null;
        this.frameCounter = 0;
        this.lastSwitchTime = 0;
        this.autoPlay = false;
        this.lSystemString = "F";
    }

    fastSin(x) {
        let t = x % (Math.PI * 2);
        if (t < 0) t += Math.PI * 2;
        let index = Math.floor((t / (Math.PI * 2)) * this.lutSize);
        return this.sinLUT[index];
    }
    fastCos(x) {
        let t = x % (Math.PI * 2);
        if (t < 0) t += Math.PI * 2;
        let index = Math.floor((t / (Math.PI * 2)) * this.lutSize);
        return this.cosLUT[index];
    }
    fastHypot(dx, dy) {
        let ax = Math.abs(dx); let ay = Math.abs(dy);
        return Math.max(ax, ay) + 0.4 * Math.min(ax, ay);
    }

    drawShape(x, y, size, colorStyle) {
        let shapeID = this.currentAnim.shape;
        let cat = shapeID % 10;
        let mod = Math.floor(shapeID / 10);
        let s = Math.max(0.5, size * (this.currentAnim.mathD * 0.2 + 0.8));

        this.ctx.save();
        this.ctx.translate(x, y);
        this.ctx.rotate(this.time * this.currentAnim.mathE * 0.1); 
        
        if (colorStyle === 'fill') {
            this.ctx.beginPath();
            if (cat === 0) { this.ctx.arc(0, 0, s, 0, Math.PI * 2); } 
            else if (cat === 1) { this.ctx.rect(-s, -s, s*2, s*(mod>5?1:2)); } 
            else if (cat === 2) { this.ctx.moveTo(0, -s); this.ctx.lineTo(s, s); this.ctx.lineTo(-s, s); this.ctx.closePath(); } 
            else if (cat === 3) { 
                let spikes = 4 + (mod % 6);
                let outer = s; let inner = s/2;
                for(let i=0; i<spikes*2; i++){
                    let r = (i%2==0) ? outer : inner; let a = (i/spikes) * Math.PI;
                    this.ctx.lineTo(Math.cos(a)*r, Math.sin(a)*r);
                }
                this.ctx.closePath();
            } 
            else if (cat === 4) { this.ctx.moveTo(0, -s); this.ctx.lineTo(s, 0); this.ctx.lineTo(0, s); this.ctx.lineTo(-s, 0); this.ctx.closePath(); } 
            else if (cat === 5) { let t = s/3; this.ctx.rect(-s, -t, s*2, t*2); this.ctx.rect(-t, -s, t*2, s*2); } 
            else if (cat === 6) { 
                let sides = 5 + (mod % 4);
                for(let i=0; i<sides; i++) {
                    let a = i * Math.PI * 2 / sides;
                    this.ctx.lineTo(Math.cos(a)*s, Math.sin(a)*s);
                }
                this.ctx.closePath();
            } 
            else if (cat === 7) { this.ctx.arc(-s, 0, s/2, Math.PI/2, Math.PI*1.5); this.ctx.arc(s, 0, s/2, Math.PI*1.5, Math.PI/2); this.ctx.closePath(); } 
            else if (cat === 8) { 
                this.ctx.moveTo(0, -s);
                this.ctx.bezierCurveTo(s, -s/2, s, s, 0, s);
                this.ctx.bezierCurveTo(-s, s, -s, -s/2, 0, -s);
                this.ctx.closePath();
            } 
            else if (cat === 9) { 
                this.ctx.moveTo(-s, -s); this.ctx.lineTo(s, s);
                this.ctx.moveTo(s, -s); this.ctx.lineTo(-s, s);
            }
            this.ctx.fill();
            if (cat === 9) this.ctx.stroke(); 
        } else {
            this.ctx.beginPath();
            if (cat === 0) { this.ctx.arc(0, 0, s, 0, Math.PI * 2); }
            else if (cat === 1) { this.ctx.rect(-s, -s, s*2, s*2); }
            else if (cat === 2) { this.ctx.moveTo(0, -s); this.ctx.lineTo(s, s); this.ctx.lineTo(-s, s); this.ctx.closePath(); }
            else { this.ctx.arc(0, 0, s, 0, Math.PI * 2); }
            this.ctx.stroke();
        }
        this.ctx.restore();
    }

    initAnimation(animId) {
        let index = animId - 1;
        if (index < 0) index = this.animations.length - 1;
        if (index >= this.animations.length) index = 0;
        this.currentAnim = JSON.parse(JSON.stringify(this.animations[index]));
        this.resetState();
    }

    resetState() {
        this.particles = [];
        this.grid = [];
        this.pegs = [];
        this.time = 0;
        this.frameCounter = 0;
        this.lastSwitchTime = Date.now();
        this.lSystemString = "F";
        
        if (!this.currentAnim) return;
        let algo = this.currentAnim.algo;
        let density = Math.max(10, this.currentAnim.density);
        
        // PARITY FIX B: C++ initialises all particle positions in V_WIDTH=256 virtual space.
        // JS must do the same so that physics runs in the same numeric range.
        // drawParticles/drawSubstrate/etc. then scale to canvas coords at draw time (like C++).
        const V = 256; // virtual coordinate space — matches C++ V_WIDTH / V_HEIGHT
        
        if ([2, 5, 7, 11, 13, 16, 20, 21, 22, 24].includes(algo)) {
            for (let i = 0; i < density; i++) {
                this.particles.push({
                    x: Math.random() * V,   // virtual space, NOT canvas pixels
                    y: Math.random() * V,
                    vx: (Math.random() - 0.5) * 2,
                    vy: (Math.random() - 0.5) * 2,
                    life: Math.random() * 100,
                    colorIdx: Math.floor(Math.random() * 256)
                });
            }
        } else if ([0, 6, 10, 12, 14, 15, 18, 19, 23].includes(algo)) {
            let cols = Math.max(4, Math.floor(Math.sqrt(density)));
            let rows = cols;
            for (let i = 0; i < cols; i++) {
                this.grid[i] = [];
                for (let j = 0; j < rows; j++) {
                    this.grid[i][j] = Math.random() > 0.5 ? 1 : 0;
                }
            }
        } else if (algo === 4) {
             this.generateLSystem(Math.max(1, Math.floor(this.currentAnim.mathC % 4) + 2));
        } else if (algo === 17) {
             let cols = Math.max(3, Math.floor(Math.sqrt(density/10)));
             for (let i = 1; i <= cols; i++) {
                 for (let j = 1; j <= cols; j++) {
                     this.pegs.push({
                         x: (i / (cols+1)) * this.width,
                         y: (j / (cols+1)) * this.height,
                         active: Math.random() > 0.3
                     });
                 }
             }
        }
    }

    updateParameter(paramName, value) {
        if (!this.currentAnim) return;
        this.currentAnim[paramName] = value;
        this.resetState();
    }

    getFormulaString() {
        if (!this.currentAnim) return "";
        let a = this.currentAnim.mathA.toFixed(2); let b = this.currentAnim.mathB.toFixed(2);
        let c = this.currentAnim.mathC.toFixed(2); let d = this.currentAnim.mathD.toFixed(2);
        let e = this.currentAnim.mathE.toFixed(2); let f = this.currentAnim.mathF.toFixed(2);
        let shp = this.currentAnim.shape; let pt = this.currentAnim.particleType; let ch = this.currentAnim.chaos.toFixed(2);
        let sym = ["None", "Horizontal", "Vertical", "Quad", "Kaleidoscope"][this.currentAnim.symmetry];
        let csp = this.currentAnim.colorSpeed.toFixed(2); let bpm = this.currentAnim.bpm; let dur = this.currentAnim.playDuration;
        let algo = this.currentAnim.algo;
        return `Algo: ${algo} | Sym: ${sym} | BPM: ${bpm} | ColSpd: ${csp} | Dur: ${dur}s\nA:${a} B:${b} C:${c} D:${d} E:${e} F:${f} Shp:${shp} PT:${pt} Ch:${ch}`;
    }
    
    // Function to parse a raw string from user and inject values
    applyFormulaString(str) {
        if (!this.currentAnim) return;
        try {
            const getVal = (label) => {
                let match = str.match(new RegExp(`${label}:\\s*([0-9\\.]+)`));
                return match ? parseFloat(match[1]) : null;
            };
            const getSym = () => {
                let match = str.match(/Sym:\s*([A-Za-z]+)/);
                if (!match) return null;
                const m = match[1];
                return ["None", "Horizontal", "Vertical", "Quad", "Kaleidoscope"].indexOf(m);
            };

            let algo = getVal("Algo"); if (algo !== null) this.currentAnim.algo = algo;
            let sym = getSym(); if (sym !== null && sym >= 0) this.currentAnim.symmetry = sym;
            let bpm = getVal("BPM"); if (bpm !== null) this.currentAnim.bpm = bpm;
            let csp = getVal("ColSpd"); if (csp !== null) this.currentAnim.colorSpeed = csp;
            let dur = getVal("Dur"); if (dur !== null) this.currentAnim.playDuration = dur;
            let a = getVal("A"); if (a !== null) this.currentAnim.mathA = a;
            let b = getVal("B"); if (b !== null) this.currentAnim.mathB = b;
            let c = getVal("C"); if (c !== null) this.currentAnim.mathC = c;
            let d = getVal("D"); if (d !== null) this.currentAnim.mathD = d;
            let e = getVal("E"); if (e !== null) this.currentAnim.mathE = e;
            let f = getVal("F"); if (f !== null) this.currentAnim.mathF = f;
            let shp = getVal("Shp"); if (shp !== null) this.currentAnim.shape = shp;
            let pt = getVal("PT"); if (pt !== null) this.currentAnim.particleType = pt;
            let ch = getVal("Ch"); if (ch !== null) this.currentAnim.chaos = ch;
            
            this.resetState();
            return true;
        } catch(e) {
            console.error("Error parsing formula", e);
            return false;
        }
    }

    update() {
        if (!this.currentAnim) return;
        
        if (this.autoPlay && Date.now() - this.lastSwitchTime > (this.currentAnim.playDuration * 1000)) {
            this.playRandomFormula();
            return; 
        }

        let timeMod = (this.currentAnim.bpm / 60.0) * this.currentAnim.speed * 0.05;
        this.time += timeMod;
        this.frameCounter++;
        
        let algo = this.currentAnim.algo;
        if (algo === 0) this.updateGameOfLife();
        else if (algo === 2) this.updateSubstrate();
        else if (algo === 5) this.updateFlowFields();
        else if (algo === 7) this.updateNeuralMorph();
        else if (algo === 11) { /* Squiggle purely drawn */ }
        else if (algo === 16) this.updateFlowFields();
        else if (algo === 20 || algo === 21 || algo === 22) this.updateNeuralMorph();
        else if (algo === 24) this.updateAIBoids();
    }

    applyLightFx() {
        let fx = this.lightFx[this.currentAnim.lightFx];
        this.ctx.globalCompositeOperation = fx.blendMode;
        if (fx.shadowBlur > 0) {
            this.ctx.shadowBlur = fx.shadowBlur;
            let cColors = this.palettes[this.currentAnim.palette].colors;
            this.ctx.shadowColor = cColors[128];
        } else {
            this.ctx.shadowBlur = 0;
        }
    }

    resetLightFx() {
        this.ctx.globalCompositeOperation = 'source-over';
        this.ctx.shadowBlur = 0;
    }

    draw() {
        if (!this.currentAnim || !this.ctx) return;
        
        let algo = this.currentAnim.algo;
        let fade = this.currentAnim.trailFade;
        
        const clearsFrame = [1, 3, 6, 8, 9, 10, 12, 13, 14, 15, 17, 18, 19, 23];
        if (clearsFrame.includes(algo)) {
            fade = 1.0;
        }
        
        this.resetLightFx();
        this.ctx.fillStyle = `rgba(0, 0, 0, ${fade})`;
        this.ctx.fillRect(0, 0, this.width, this.height);
        
        this.applyLightFx();
        
        let sym = this.currentAnim.symmetry;
        if (sym > 0) {
            if (!this.offCtx) {
                let offCanvas = document.createElement('canvas');
                offCanvas.width = this.width;
                offCanvas.height = this.height;
                this.offCtx = offCanvas.getContext('2d');
            }
            this.offCtx.clearRect(0,0,this.width, this.height);
            let realCtx = this.ctx;
            this.ctx = this.offCtx;
            this.drawAlgorithms(algo); 
            this.ctx = realCtx; 
            
            this.ctx.save();
            if (sym === 1) { 
                this.ctx.drawImage(this.offCtx.canvas, 0, 0, this.width, this.height/2, 0, 0, this.width, this.height/2);
                this.ctx.scale(1, -1);
                this.ctx.drawImage(this.offCtx.canvas, 0, 0, this.width, this.height/2, 0, -this.height, this.width, this.height/2);
            } else if (sym === 2) { 
                this.ctx.drawImage(this.offCtx.canvas, 0, 0, this.width/2, this.height, 0, 0, this.width/2, this.height);
                this.ctx.scale(-1, 1);
                this.ctx.drawImage(this.offCtx.canvas, 0, 0, this.width/2, this.height, -this.width, 0, this.width/2, this.height);
            } else if (sym === 3) { 
                this.ctx.drawImage(this.offCtx.canvas, 0, 0, this.width/2, this.height/2, 0, 0, this.width/2, this.height/2);
                this.ctx.scale(-1, 1); this.ctx.drawImage(this.offCtx.canvas, 0, 0, this.width/2, this.height/2, -this.width, 0, this.width/2, this.height/2);
                this.ctx.scale(1, -1); this.ctx.drawImage(this.offCtx.canvas, 0, 0, this.width/2, this.height/2, -this.width, -this.height, this.width/2, this.height/2);
                this.ctx.scale(-1, 1); this.ctx.drawImage(this.offCtx.canvas, 0, 0, this.width/2, this.height/2, 0, -this.height, this.width/2, this.height/2);
            } else if (sym === 4) { 
                this.ctx.translate(this.width/2, this.height/2);
                let slices = 8;
                for(let i=0; i<slices; i++) {
                    this.ctx.rotate((Math.PI*2)/slices);
                    this.ctx.drawImage(this.offCtx.canvas, 0, 0, this.width/2, this.height/2, 0, 0, this.width/2, this.height/2);
                    this.ctx.scale(-1, 1);
                    this.ctx.drawImage(this.offCtx.canvas, 0, 0, this.width/2, this.height/2, 0, 0, this.width/2, this.height/2);
                    this.ctx.scale(-1, 1); 
                }
            }
            this.ctx.restore();
            
        } else {
            this.drawAlgorithms(algo);
        }
        
        this.resetLightFx();
    }

    drawAlgorithms(algo) {
        if (algo === 0) this.drawGameOfLife();
        else if (algo === 1) this.drawMandelbrot();
        else if (algo === 2) this.drawSubstrate();
        else if (algo === 3) this.drawFractalFlame();
        else if (algo === 4) this.drawLSystem();
        else if (algo === 5) this.drawParticles();
        else if (algo === 6) this.drawPlasma();
        else if (algo === 7) this.drawNeuralMorph();
        else if (algo === 8) this.drawPlotter();
        else if (algo === 9) this.drawLEDPulse();
        else if (algo === 10) this.drawGeometric();
        else if (algo === 11) this.drawSquiggle();
        else if (algo === 12) this.drawMolnar();
        else if (algo === 13) this.drawNake();
        else if (algo === 14) this.drawNees();
        else if (algo === 15) this.drawLeWitt();
        else if (algo === 16) this.drawFidenza();
        else if (algo === 17) this.drawRingers();
        else if (algo === 18) this.drawAutoglyphs();
        else if (algo === 19) this.drawArchetype();
        else if (algo === 20) this.drawPassersby();
        else if (algo === 21) this.drawAnadol();
        else if (algo === 22) this.drawLearningToSee();
        else if (algo === 23) this.drawAvidLines();
        else if (algo === 24) this.drawParticles();
    }

    playRandomFormula() {
        let id = Math.floor(Math.random() * this.animations.length) + 1;
        this.initAnimation(id);
    }

    getColor(index) {
        let cColors = this.palettes[this.currentAnim.palette].colors;
        let finalIndex = index + (this.time * this.currentAnim.colorSpeed * 50);
        let safeIdx = Math.floor(Math.abs(finalIndex)) % 256;
        return cColors[safeIdx];
    }

    // --- ALGORITHMS ---
    updateGameOfLife() {
        let speedMod = Math.max(1, Math.floor(10 / this.currentAnim.speed));
        if (this.frameCounter % speedMod !== 0) return;
        let cols = this.grid.length;
        if(cols===0) return;
        let rows = this.grid[0].length;
        let next = Array(cols).fill().map(() => Array(rows).fill(0));
        for (let i = 0; i < cols; i++) {
            for (let j = 0; j < rows; j++) {
                let state = this.grid[i][j];
                let neighbors = 0;
                for (let x = -1; x < 2; x++) {
                    for (let y = -1; y < 2; y++) {
                        neighbors += this.grid[(i + x + cols) % cols][(j + y + rows) % rows];
                    }
                }
                neighbors -= state;
                let mutation = Math.random() < (this.currentAnim.mathA * 0.05 + this.currentAnim.chaos * 0.01);
                if (state == 0 && (neighbors == 3 || mutation)) next[i][j] = 1;
                else if (state == 1 && (neighbors < 2 || neighbors > 3)) next[i][j] = 0;
                else next[i][j] = state;
            }
        }
        this.grid = next;
    }
    drawGameOfLife() {
        let cols = this.grid.length;
        if(cols===0) return;
        let rows = this.grid[0].length;
        let cellW = this.width / cols;
        let cellH = this.height / rows;
        for (let i = 0; i < cols; i++) {
            for (let j = 0; j < rows; j++) {
                if (this.grid[i][j] == 1) {
                    this.ctx.fillStyle = this.getColor((i/cols + j/rows)*128);
                    let cx = i * cellW + cellW/2;
                    let cy = j * cellH + cellH/2;
                    this.drawShape(cx, cy, cellW/2, 'fill');
                }
            }
        }
    }

    drawMandelbrot() {
        let maxIter = Math.floor(10 + this.currentAnim.mathC);
        let zoom = 1.0 + this.fastSin(this.time * 0.1) * 0.5 + this.currentAnim.mathB;
        let moveX = this.fastCos(this.time * 0.2) * 0.5 + this.currentAnim.mathD - 5.0;
        let moveY = this.fastSin(this.time * 0.2) * 0.5 + this.currentAnim.mathE - 5.0;
        let res = 4;
        let chaos = this.currentAnim.chaos * 0.1;
        for (let x = 0; x < this.width; x+=res) {
            for (let y = 0; y < this.height; y+=res) {
                let pr = 1.5 * (x - this.width / 2) / (0.5 * zoom * this.width) + moveX + (Math.random()*chaos);
                let pi = (y - this.height / 2) / (0.5 * zoom * this.height) + moveY + (Math.random()*chaos);
                let newRe = 0, newIm = 0, oldRe = 0, oldIm = 0;
                let i;
                for (i = 0; i < maxIter; i++) {
                    oldRe = newRe; oldIm = newIm;
                    newRe = oldRe * oldRe - oldIm * oldIm + pr + this.currentAnim.mathA;
                    newIm = 2 * oldRe * oldIm + pi;
                    if ((newRe * newRe + newIm * newIm) > 4) break;
                }
                if (i < maxIter) {
                    this.ctx.fillStyle = this.getColor(i * 255 / maxIter);
                    this.ctx.fillRect(x, y, res, res);
                }
            }
        }
    }

    updateSubstrate() {
        let a = this.currentAnim.mathA; let b = this.currentAnim.mathB;
        let seedChance = 0.01 * this.currentAnim.mathD + (this.currentAnim.chaos * 0.005);
        for (let p of this.particles) {
            let angle = this.fastSin(p.x * a) * this.fastCos(p.y * b) * Math.PI * 2 + this.time;
            p.vx = this.fastCos(angle) * this.currentAnim.speed * this.currentAnim.mathC + (Math.random()*this.currentAnim.chaos - this.currentAnim.chaos/2)*0.1;
            p.vy = this.fastSin(angle) * this.currentAnim.speed * this.currentAnim.mathE + (Math.random()*this.currentAnim.chaos - this.currentAnim.chaos/2)*0.1;
            p.x += p.vx; p.y += p.vy;
            // PARITY FIX A: advance colorIdx each frame to match C++ engine ("Fix #2")
            p.colorIdx = (p.colorIdx + 1) % 256;
            // PARITY FIX B: respawn in virtual 0-256 space
            if (Math.random() < seedChance) { p.x = Math.random() * 256; p.y = Math.random() * 256; }
        }
    }
    drawSubstrate() {
        // PARITY FIX B: particles live in 0-256 virtual space; scale to canvas
        const scaleX = this.width / 256, scaleY = this.height / 256;
        for (let p of this.particles) {
            this.ctx.fillStyle = this.getColor(p.colorIdx);
            this.ctx.fillRect(p.x * scaleX, p.y * scaleY, 2, 2);
        }
        for (let i = 0; i < this.particles.length; i+=10) {
            let p1 = this.particles[i]; let p2 = this.particles[(i+1)%this.particles.length];
            let dist = this.fastHypot(p1.x-p2.x, p1.y-p2.y);
            if (dist < 30 * this.currentAnim.mathC) {
                this.ctx.strokeStyle = this.getColor(p1.colorIdx);
                this.ctx.beginPath(); this.ctx.moveTo(p1.x * scaleX, p1.y * scaleY); this.ctx.lineTo(p2.x * scaleX, p2.y * scaleY); this.ctx.stroke();
            }
        }
    }

    drawFractalFlame() {
        let iters = Math.floor(1000 * this.currentAnim.density / 100);
        let x = 0, y = 0;
        let a = this.currentAnim.mathA; let b = this.currentAnim.mathB; let mC = this.currentAnim.mathC;
        let shift = this.currentAnim.mathD * 20;
        let chaos = this.currentAnim.chaos * 0.05;
        for(let i=0; i<iters; i++) {
            let r = Math.random();
            let nx, ny;
            if (r < 0.33) {
                nx = this.fastSin(x*a) - this.fastCos(y*b) + (Math.random()*chaos - chaos/2);
                ny = this.fastSin(y*a) - this.fastCos(x*b) + (Math.random()*chaos - chaos/2);
            } else if (r < 0.66) {
                nx = x*this.fastCos(this.time) - y*this.fastSin(this.time) + mC;
                ny = x*this.fastSin(this.time) + y*this.fastCos(this.time) - mC;
            } else {
                nx = x*a - y*a; ny = x*b + y*b;
            }
            x = nx; y = ny;
            let px = this.width/2 + x * 20 * this.currentAnim.complexity;
            let py = this.height/2 + y * 20 * this.currentAnim.complexity;
            this.ctx.fillStyle = this.getColor((i % 256) + shift);
            this.ctx.fillRect(px, py, 1, 1);
        }
    }

    generateLSystem(iters) {
        let axiom = "F"; let rule = "F[+F]F[-F]F";
        for (let i = 0; i < iters; i++) {
            let next = "";
            for (let char of axiom) { if (char === "F") next += rule; else next += char; }
            axiom = next;
        }
        this.lSystemString = axiom;
    }
    drawLSystem() {
        this.ctx.save();
        this.ctx.translate(this.width/2, this.height);
        let len = 5 * this.currentAnim.mathB;
        let angle = this.currentAnim.mathA * Math.PI + this.fastSin(this.time*0.5)*this.currentAnim.mathE + (this.currentAnim.chaos*0.01); 
        let cIdx = 0;
        this.ctx.lineWidth = this.currentAnim.mathD;
        for (let char of this.lSystemString) {
            if (char === "F") {
                this.ctx.strokeStyle = this.getColor(cIdx);
                this.ctx.beginPath(); this.ctx.moveTo(0,0); this.ctx.lineTo(0, -len); this.ctx.stroke();
                this.ctx.translate(0, -len);
                cIdx++;
            } else if (char === "+") { this.ctx.rotate(angle);
            } else if (char === "-") { this.ctx.rotate(-angle);
            } else if (char === "[") { this.ctx.save();
            } else if (char === "]") { this.ctx.restore(); }
        }
        this.ctx.restore();
    }

    updateFlowFields() {
        let c = this.currentAnim.complexity; let a = this.currentAnim.mathA; let b = this.currentAnim.mathB;
        let chaos = this.currentAnim.chaos;
        let pt = this.currentAnim.particleType % 3;
        
        for (let p of this.particles) {
            let angle = this.fastSin(p.x * 0.01 * a * c) * this.fastCos(p.y * 0.01 * b * c) * Math.PI * 2 + this.time * this.currentAnim.mathC;
            
            if (pt === 0) { 
                p.vx = this.fastCos(angle) * this.currentAnim.speed + (Math.random()*chaos - chaos/2)*0.2;
                p.vy = this.fastSin(angle) * this.currentAnim.speed + (Math.random()*chaos - chaos/2)*0.2;
            } else if (pt === 1) { 
                p.vx += (Math.random()-0.5) * chaos * 0.5;
                p.vy += (Math.random()-0.5) * chaos * 0.5;
                p.vx *= 0.95; p.vy *= 0.95; 
            } else if (pt === 2) { 
                p.vy += 0.1 * this.currentAnim.speed; 
                p.vx += (Math.random()-0.5) * chaos * 0.1;
                // PARITY FIX B: bounce against virtual height 256, not canvas height
                if (p.y > 256) { p.vy *= -0.8; p.y = 256; }
            }
            
            p.x += p.vx; p.y += p.vy;
            // PARITY FIX B: wrap in virtual 0-256 space (matches C++ V_WIDTH/V_HEIGHT)
            if (p.x < 0) p.x += 256; if (p.x > 256) p.x -= 256;
            if (p.y < 0) p.y += 256; if (p.y > 256) p.y -= 256;
            // PARITY FIX A: advance colorIdx each frame to match C++ engine ("Fix #2")
            p.colorIdx = (p.colorIdx + 1) % 256;
        }
    }
    
    updateAIBoids() {
        let chaos = this.currentAnim.chaos;
        let speed = this.currentAnim.speed;
        let sepDist = 20 * this.currentAnim.mathA;
        let alignDist = 40 * this.currentAnim.mathB;
        let cohDist = 40 * this.currentAnim.mathC;
        
        for (let i = 0; i < this.particles.length; i++) {
            let p = this.particles[i];
            let sepX = 0, sepY = 0;
            let aliX = 0, aliY = 0, aliCount = 0;
            let cohX = 0, cohY = 0, cohCount = 0;
            
            for (let j = 0; j < this.particles.length; j++) {
                if(i === j) continue;
                let other = this.particles[j];
                let d = this.fastHypot(p.x - other.x, p.y - other.y);
                
                if (d < sepDist && d > 0) {
                    sepX += (p.x - other.x) / d; sepY += (p.y - other.y) / d;
                }
                if (d < alignDist) {
                    aliX += other.vx; aliY += other.vy; aliCount++;
                }
                if (d < cohDist) {
                    cohX += other.x; cohY += other.y; cohCount++;
                }
            }
            
            p.vx += sepX * 0.1; p.vy += sepY * 0.1;
            if (aliCount > 0) {
                p.vx += ((aliX / aliCount) - p.vx) * 0.05;
                p.vy += ((aliY / aliCount) - p.vy) * 0.05;
            }
            if (cohCount > 0) {
                p.vx += (((cohX / cohCount) - p.x) * 0.01);
                p.vy += (((cohY / cohCount) - p.y) * 0.01);
            }
            
            p.vx += (Math.random()-0.5) * chaos * 0.2;
            p.vy += (Math.random()-0.5) * chaos * 0.2;
            
            let mag = this.fastHypot(p.vx, p.vy) + 0.01;
            if(mag > speed*2) {
                p.vx = (p.vx/mag)*speed*2;
                p.vy = (p.vy/mag)*speed*2;
            }
            
            p.x += p.vx; p.y += p.vy;
            // PARITY FIX B: wrap in virtual 0-256 space (matches C++ V_WIDTH/V_HEIGHT)
            if (p.x < 0) p.x += 256; if (p.x > 256) p.x -= 256;
            if (p.y < 0) p.y += 256; if (p.y > 256) p.y -= 256;
            // PARITY FIX A: advance colorIdx each frame to match C++ engine ("Fix #2")
            p.colorIdx = (p.colorIdx + 1) % 256;
        }
    }

    drawParticles() {
        // PARITY FIX B: particles live in 0-256 virtual space; scale to canvas
        const scaleX = this.width / 256, scaleY = this.height / 256;
        const scale  = Math.min(scaleX, scaleY);
        for (let p of this.particles) {
            this.ctx.fillStyle = this.getColor(p.colorIdx);
            this.drawShape(p.x * scaleX, p.y * scaleY, 2 * scale, 'fill');
        }
    }

    drawPlasma() {
        let a = this.currentAnim.mathA; let b = this.currentAnim.mathB; let c = this.currentAnim.mathC;
        let gridX = Math.max(1, Math.floor(4 * this.currentAnim.mathD));
        let gridY = Math.max(1, Math.floor(4 * this.currentAnim.mathE));
        let chaos = this.currentAnim.chaos * 0.1;
        for (let y = 0; y < this.height; y += gridY) {
            for (let x = 0; x < this.width; x += gridX) {
                let v = this.fastSin(x*0.01*a + this.time) + this.fastSin(y*0.01*b + this.time) + this.fastSin((x+y)*0.01*c) + (Math.random()*chaos);
                this.ctx.fillStyle = this.getColor((v + 3) / 6 * 255);
                this.ctx.fillRect(x, y, gridX, gridY);
            }
        }
    }

    updateNeuralMorph() { this.updateFlowFields(); }
    drawNeuralMorph() {
        // PARITY FIX B: particles live in 0-256 virtual space; scale to canvas
        const scaleX = this.width / 256, scaleY = this.height / 256;
        const scale  = Math.min(scaleX, scaleY);
        let a = this.currentAnim.mathA; let b = this.currentAnim.mathB;
        for (let p of this.particles) {
            this.ctx.fillStyle = this.getColor(p.colorIdx);
            let s = Math.max(0.5, Math.abs(this.fastSin(this.time + p.x)*5*this.currentAnim.mathE) + this.currentAnim.mathD) * scale;
            this.ctx.beginPath(); this.ctx.arc(p.x * scaleX, p.y * scaleY, s, 0, Math.PI * 2); this.ctx.fill();
        }
        let maxDist = 50 * a * b;
        for(let i=0; i<this.particles.length; i++) {
            for(let j=i+1; j<this.particles.length; j++) {
                let p1 = this.particles[i]; let p2 = this.particles[j];
                let d = this.fastHypot(p1.x-p2.x, p1.y-p2.y);
                if(d < maxDist) {
                    this.ctx.strokeStyle = this.getColor(p1.colorIdx);
                    this.ctx.globalAlpha = 1.0 - (d/maxDist);
                    this.ctx.beginPath(); this.ctx.moveTo(p1.x * scaleX, p1.y * scaleY); this.ctx.lineTo(p2.x * scaleX, p2.y * scaleY); this.ctx.stroke();
                }
            }
        }
        this.ctx.globalAlpha = 1.0;
    }

    drawPlotter() {
        let step = Math.max(5, 40 - this.currentAnim.density / 25);
        let a = this.currentAnim.mathA; let b = this.currentAnim.mathB; let c = this.currentAnim.mathC;
        let thresh = this.currentAnim.mathD; let amp = this.currentAnim.mathE;
        for (let x = 0; x < this.width; x += step) {
            for (let y = 0; y < this.height; y += step) {
                let noise = this.fastSin(x*a + y*b + this.time);
                if (noise > this.fastSin(c) * thresh) {
                    this.ctx.strokeStyle = this.getColor((x/this.width)*128 + (y/this.height)*128);
                    this.ctx.strokeRect(x + noise*5*amp, y, Math.max(1, step-2), Math.max(1, step-2));
                }
            }
        }
    }

    drawLEDPulse() {
        let a = this.currentAnim.mathA; let b = this.currentAnim.mathB; let c = this.currentAnim.mathC;
        for(let x=0; x<this.width; x+=10) {
            for(let y=0; y<this.height; y+=10) {
                let pulse = (this.fastSin(this.time * a + x * 0.05 * b + y * 0.05 * c) + 1) / 2;
                this.ctx.fillStyle = this.getColor(pulse * 255);
                this.ctx.globalAlpha = pulse;
                this.drawShape(x+5, y+5, pulse*4 + 1, 'fill');
            }
        }
        this.ctx.globalAlpha = 1.0;
    }

    drawGeometric() {
        let cells = this.currentAnim.complexity > 5 ? 8 : 4;
        let cellSize = this.width / cells;
        let a = this.currentAnim.mathA; let b = this.currentAnim.mathB; let c = this.currentAnim.mathC;
        let chaos = this.currentAnim.chaos * 0.5;
        for (let cy = 0; cy < cells; cy++) {
            for (let cx = 0; cx < cells; cx++) {
                let n = this.fastSin(cx * a + cy * b + this.time * c) + (Math.random()*chaos);
                this.ctx.fillStyle = this.getColor(Math.abs(n) * 255);
                this.drawShape(cx*cellSize + cellSize/2, cy*cellSize + cellSize/2, Math.max(1, cellSize/2), 'fill');
            }
        }
    }

    drawSquiggle() {
        this.ctx.lineWidth = this.currentAnim.mathF * 5 + 1;
        let a = this.currentAnim.mathA; let b = this.currentAnim.mathB; let c = this.currentAnim.mathC;
        let amp = this.currentAnim.mathD * 10;
        let freq = this.currentAnim.mathE;
        let chaos = this.currentAnim.chaos;
        for (let yOffset = 0; yOffset < this.height; yOffset += 20) {
            this.ctx.strokeStyle = this.getColor(yOffset / this.height * 255);
            this.ctx.beginPath();
            for (let x = 0; x <= this.width; x += 10) {
                let y = yOffset + this.fastSin(x * 0.05 * a * freq + this.time * b) * (amp * c) + (Math.random()*chaos);
                if (x === 0) this.ctx.moveTo(x, y); else this.ctx.lineTo(x, y);
            }
            this.ctx.stroke();
        }
    }

    drawMolnar() {
        let cols = Math.max(2, Math.floor(16 * this.currentAnim.mathA));
        let cellW = this.width / cols; let cellH = this.height / cols;
        for(let i=0; i<cols; i++){
            for(let j=0; j<cols; j++){
                this.ctx.save();
                this.ctx.translate(i*cellW + cellW/2, j*cellH + cellH/2);
                let rot = this.fastSin(this.time + i*this.currentAnim.mathB + j) * Math.PI/4 * this.currentAnim.mathE;
                this.ctx.rotate(rot + (Math.random() * this.currentAnim.chaos * 0.1));
                this.ctx.strokeStyle = this.getColor((i*j)*10);
                this.ctx.lineWidth = this.currentAnim.mathD * 5;
                let size = Math.max(1, cellW * 0.8 * Math.max(0.1, this.currentAnim.mathC));
                this.ctx.strokeRect(-size/2, -size/2, size, size);
                this.ctx.restore();
            }
        }
    }

    drawNake() {
        let len = 50 * this.currentAnim.mathA;
        let chaos = this.currentAnim.mathB + this.currentAnim.chaos * 0.1;
        let num = Math.floor(this.currentAnim.density / 2);
        for(let i=0; i<num; i++){
            let x1 = (this.fastSin(this.time + i) * 0.5 + 0.5) * this.width;
            let y1 = (this.fastCos(this.time + i*chaos) * 0.5 + 0.5) * this.height;
            this.ctx.strokeStyle = this.getColor(i % 256);
            this.ctx.fillStyle = this.getColor(i % 256);
            this.drawShape(x1, y1, Math.max(1, len/4), 'stroke');
        }
    }

    drawNees() {
        let cols = 10;
        let cellW = this.width / cols; let cellH = this.height / cols;
        for(let i=0; i<cols; i++){
            for(let j=0; j<cols; j++){
                let dx = (Math.random()-0.5) * j * this.currentAnim.mathA * (1+this.currentAnim.chaos*0.1);
                let dy = (Math.random()-0.5) * j * this.currentAnim.mathB * (1+this.currentAnim.chaos*0.1);
                this.ctx.strokeStyle = this.getColor(j*25);
                this.drawShape(i*cellW + dx + cellW/2, j*cellH + dy + cellH/2, Math.max(1, (cellW-4)/2 * Math.max(0.1, this.currentAnim.mathC)), 'stroke');
            }
        }
    }

    drawLeWitt() {
        let pts = [];
        let seed = this.currentAnim.mathC;
        for(let i=0; i<50; i++){
            let x = (this.fastSin(i*seed + this.time*0.1)*0.5+0.5) * this.width * this.currentAnim.mathD;
            let y = (this.fastCos(i*seed + this.time*0.1)*0.5+0.5) * this.height * this.currentAnim.mathE;
            x += (Math.random()-0.5) * this.currentAnim.chaos * 10;
            y += (Math.random()-0.5) * this.currentAnim.chaos * 10;
            pts.push({x, y});
        }
        for(let i=0; i<pts.length-1; i++){
            this.ctx.strokeStyle = this.getColor(i * 5);
            this.ctx.beginPath();
            this.ctx.moveTo(pts[i].x, pts[i].y);
            this.ctx.lineTo(pts[i+1].x, pts[i+1].y);
            this.ctx.stroke();
        }
    }

    drawFidenza() {
        // PARITY FIX B: particles live in 0-256 virtual space; scale to canvas
        const scaleX = this.width / 256, scaleY = this.height / 256;
        const scale  = Math.min(scaleX, scaleY);
        for (let p of this.particles) {
            this.ctx.fillStyle = this.getColor(p.colorIdx);
            let baseW = this.currentAnim.mathC * 5 * scale;
            let varW = this.fastSin(p.x * 0.05)*4 * this.currentAnim.mathD * scale;
            let width = Math.max(scale, baseW + varW);
            this.drawShape(p.x * scaleX, p.y * scaleY, width, 'fill');
        }
    }

    drawRingers() {
        for(let p of this.pegs) {
            this.ctx.fillStyle = this.getColor(0);
            this.ctx.beginPath(); this.ctx.arc(p.x, p.y, Math.max(1, 4 * this.currentAnim.mathD), 0, Math.PI*2); this.ctx.fill();
        }
        this.ctx.strokeStyle = this.getColor(128);
        this.ctx.lineWidth = Math.max(1, 2 * this.currentAnim.mathE);
        this.ctx.beginPath();
        let first = true;
        let chaosOff = this.currentAnim.chaos * 2;
        for(let p of this.pegs) {
            if(p.active) {
                let ox = (Math.random()-0.5) * chaosOff;
                let oy = (Math.random()-0.5) * chaosOff;
                if(first) { this.ctx.moveTo(p.x+ox, p.y+oy); first = false; }
                else { this.ctx.lineTo(p.x+ox, p.y+oy); }
            }
        }
        this.ctx.stroke();
    }

    drawAutoglyphs() {
        let cols = Math.max(8, Math.floor(16 * this.currentAnim.mathA));
        let cellW = this.width / cols; let cellH = this.height / cols;
        for(let i=0; i<cols; i++){
            for(let j=0; j<cols; j++){
                let v = this.fastSin(i * j * this.currentAnim.mathB);
                if(Math.random() < this.currentAnim.chaos * 0.05) v *= -1; 
                this.ctx.strokeStyle = this.getColor((i+j)*10);
                this.ctx.lineWidth = Math.max(1, 2 * this.currentAnim.mathD);
                this.ctx.beginPath();
                if(v > 0.5 * this.currentAnim.mathC) { this.ctx.moveTo(i*cellW, j*cellH); this.ctx.lineTo((i+1)*cellW, (j+1)*cellH); }
                else if(v < -0.5 * this.currentAnim.mathC) { this.ctx.moveTo((i+1)*cellW, j*cellH); this.ctx.lineTo(i*cellW, (j+1)*cellH); }
                else { this.ctx.moveTo(i*cellW + cellW/2, j*cellH); this.ctx.lineTo(i*cellW + cellW/2, (j+1)*cellH); }
                this.ctx.stroke();
            }
        }
    }

    drawArchetype() {
        // PARITY FIX D: C++ scales pad/gap by scaleX and works in screen pixels.
        // JS simulator runs at canvas size (=screen size), so scaleX = width/256.
        // To match C++: pad = mathD*10*(width/256), gap = mathE*5*(width/256)
        let scaleX = this.width / 256;
        let divX = this.width * (0.3 + this.fastSin(this.time)*0.2 * this.currentAnim.mathA);
        let divY = this.height * (0.5 + this.fastCos(this.time)*0.3);
        let pad = this.currentAnim.mathD * 10 * scaleX;
        let gap = this.currentAnim.mathE * 5 * scaleX;
        this.ctx.fillStyle = this.getColor(50); this.ctx.fillRect(pad, pad, divX-gap, divY-gap);
        this.ctx.fillStyle = this.getColor(100); this.ctx.fillRect(divX+gap, pad, this.width-divX-pad-gap, divY-gap);
        this.ctx.fillStyle = this.getColor(150); this.ctx.fillRect(pad, divY+gap, divX-gap, this.height-divY-pad-gap);
        this.ctx.fillStyle = this.getColor(200); this.ctx.fillRect(divX+gap, divY+gap, this.width-divX-pad-gap, this.height-divY-pad-gap);
    }

    drawPassersby() {
        this.drawNeuralMorph();
        // PARITY FIX C: match C++ engine which halves pixel brightness outside the radius
        // (C++: pixel >>= 1 for vignetteOutside pixels).
        // JS canvas has no per-pixel read-back in a tight loop, so we approximate with
        // a radial gradient overlay at 50% opacity — same visual darkening, same edge.
        let focus = this.currentAnim.mathB;
        let r = (this.width / 2.5) * focus;
        let cx = this.width / 2, cy = this.height / 2;
        let grad = this.ctx.createRadialGradient(cx, cy, r * 0.95, cx, cy, r * 1.05);
        grad.addColorStop(0, 'rgba(0,0,0,0)');
        grad.addColorStop(1, 'rgba(0,0,0,0.5)');
        // Fill entire canvas with the gradient, then cover outside solidly
        this.ctx.save();
        this.ctx.fillStyle = 'rgba(0,0,0,0.5)';
        this.ctx.beginPath();
        this.ctx.arc(cx, cy, r, 0, Math.PI * 2);
        this.ctx.rect(this.width, 0, -this.width, this.height);
        this.ctx.fill();
        this.ctx.restore();
    }

    drawAnadol() {
        // PARITY FIX B: particles live in 0-256 virtual space; scale to canvas
        const scaleX = this.width / 256, scaleY = this.height / 256;
        const scale  = Math.min(scaleX, scaleY);
        for (let p of this.particles) {
            let rad = Math.max(scale, (10 + this.fastSin(p.x * 0.05 + this.time)*5) * this.currentAnim.mathC * scale);
            this.ctx.fillStyle = this.getColor(p.colorIdx);
            this.ctx.globalAlpha = 0.3 * this.currentAnim.mathD;
            this.drawShape(p.x * scaleX, p.y * scaleY, rad, 'fill');
        }
        this.ctx.globalAlpha = 1.0;
    }

    drawLearningToSee() {
        // PARITY FIX B: particles live in 0-256 virtual space; scale to canvas
        const scaleX = this.width / 256, scaleY = this.height / 256;
        const scale  = Math.min(scaleX, scaleY);
        for (let p of this.particles) {
            let r = Math.max(1, this.fastSin(p.x*0.1 + p.y*0.1 + this.time) > (0.5 * this.currentAnim.mathD) ? 5 * scale : 1 * scale);
            this.ctx.fillStyle = this.getColor(p.colorIdx);
            this.ctx.fillRect(p.x * scaleX, p.y * scaleY, r, r);
        }
    }

    drawAvidLines() {
        let lines = Math.max(1, Math.floor(10 * Math.max(0.1, this.currentAnim.mathA)));
        for(let l=0; l<lines; l++) {
            this.ctx.strokeStyle = this.getColor(l * 20);
            this.ctx.lineWidth = 1.5;
            this.ctx.beginPath();
            for(let x=0; x<=this.width; x+=5) {
                let y = (l/lines)*this.height * this.currentAnim.mathD + this.fastSin(x*0.02 + l*0.5 + this.time)*20*this.currentAnim.mathB;
                y += (Math.random()-0.5) * this.currentAnim.chaos * 2;
                if(x===0) this.ctx.moveTo(x,y); else this.ctx.lineTo(x,y);
            }
            this.ctx.stroke();
        }
    }

    exportConfigC() {
        let a = this.currentAnim;
        let safeName = a.name.replace(/\s+/g, '_').toUpperCase();
        let str = `// Auto-generated C++ Configuration for ESP32 Super Art Engine
// Formula Preset: ${a.name}

#ifndef SUPER_ART_CONFIG_${a.id}_H
#define SUPER_ART_CONFIG_${a.id}_H

struct ArtConfig {
    int id;
    int algo;
    int palette;
    int lightFx;
    int shape;
    int particleType;
    int symmetry;
    int bpm;
    int playDuration;
    float colorSpeed;
    float chaos;
    float speed;
    float density;
    float complexity;
    float trailFade;
    float mathA;
    float mathB;
    float mathC;
    float mathD;
    float mathE;
    float mathF;
};

const ArtConfig CONFIG_${safeName} = {
    ${a.id}, // id
    ${a.algo}, // algo
    ${a.palette}, // palette
    ${a.lightFx}, // lightFx
    ${a.shape}, // shape
    ${a.particleType}, // particleType
    ${a.symmetry}, // symmetry
    ${a.bpm}, // bpm
    ${a.playDuration}, // playDuration
    ${a.colorSpeed}f, // colorSpeed
    ${a.chaos}f, // chaos
    ${a.speed}f, // speed
    ${a.density}f, // density
    ${a.complexity}f, // complexity
    ${a.trailFade}f, // trailFade
    ${a.mathA}f, // mathA
    ${a.mathB}f, // mathB
    ${a.mathC}f, // mathC
    ${a.mathD}f, // mathD
    ${a.mathE}f, // mathE
    ${a.mathF}f  // mathF
};

#endif // SUPER_ART_CONFIG_${a.id}_H
`;
        return str;
    }

    exportConfigJson() {
        return JSON.stringify(this.currentAnim, null, 4);
    }
    
    loadConfigJson(jsonStr) {
        try {
            let parsed = JSON.parse(jsonStr);
            if (parsed && typeof parsed.algo !== 'undefined') {
                this.currentAnim = parsed;
                this.resetState();
                return true;
            }
        } catch (e) {
            console.error("Failed to load JSON", e);
        }
        return false;
    }
}

if (typeof module !== 'undefined') {
    module.exports = SuperArtEngine;
}
