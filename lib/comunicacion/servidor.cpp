#include "servidor.h"
#include "config.h"
#include "generador.h"
#include <WiFi.h>
#include <WebServer.h>

// El main expone estas funciones
extern int modo_activo;
extern void cambiar_modo(int nuevo);
extern String leer_medicion();         // valor puntual ya formateado
extern String capturar_osciloscopio(); // bloque de muestras en JSON

static WebServer server(80);

static const char *AP_SSID = "Multimetr Mayra";
static const char *AP_PASS = "12345678"; // minimo 8 caracteres

static const char PAGINA[] PROGMEM = R"HTML(
<!DOCTYPE html><html><head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Multimetro</title>
<style>
 body{font-family:sans-serif;margin:0;padding:16px;background:#f2f2f2;color:#222}
 h1{font-size:20px;font-weight:500;text-align:center;margin:8px 0 16px}
 #valor{font-size:44px;text-align:center;padding:24px 8px;background:#fff;
        border-radius:12px;margin-bottom:16px;font-variant-numeric:tabular-nums}
 .modos{display:grid;grid-template-columns:1fr 1fr;gap:10px}
 button{padding:16px;font-size:16px;border:none;border-radius:10px;
        background:#fff;color:#222;cursor:pointer}
 button.activo{background:#2b6cb0;color:#fff}
 #estado{text-align:center;font-size:13px;color:#666;margin-top:14px}
 #multipanel{display:none}
 #genpanel{display:none;background:#fff;border-radius:12px;padding:14px;margin-top:10px}
 .ondas{display:grid;grid-template-columns:1fr 1fr 1fr;gap:8px;margin-bottom:6px}
 .ondas button{padding:12px 4px;font-size:14px}
 .ctrl{margin:14px 0}
 .ctrl .lab{display:flex;justify-content:space-between;font-size:13px;color:#555;margin-bottom:6px}
 .ctrl input[type=range]{width:100%}
 .genfila{display:flex;gap:8px;align-items:center}
 .genfila input[type=range]{flex:1}
 .genfila input[type=number]{width:88px;padding:8px;border:1px solid #ccc;border-radius:8px;
        font-size:14px;text-align:center}
</style></head><body>
<h1>Multimetro</h1>
<div style="margin-bottom:10px">
  <button onclick="toggleMulti()" id="bmulti" style="width:100%">Multímetro</button>
</div>
<div id="multipanel">
  <div id="valor">--</div>
  <div class="modos">
    <button onclick="setModo(1)" id="b1">Corriente</button>
    <button onclick="setModo(2)" id="b2">Voltaje</button>
    <button onclick="setModo(3)" id="b3">Resistencia</button>
    <button onclick="setModo(4)" id="b4">Capacitancia</button>
    <button onclick="setModo(5)" id="b5">Frecuencia</button>
    <button onclick="setModo(6)" id="b6">Continuidad</button>
  </div>
</div>
<div style="margin-top:14px">
  <button onclick="toggleOsc()" id="bosc" style="width:100%">Osciloscopio</button>
</div>
<canvas id="osc" width="320" height="180"
        style="width:100%;background:#fff;border-radius:10px;margin-top:10px;display:none"></canvas>

<div style="margin-top:10px">
  <button onclick="toggleGen()" id="bgen" style="width:100%">Generador</button>
</div>
<div id="genpanel">
  <div class="ondas">
    <button onclick="setOnda('sine')" id="w_sine" class="activo">Seno</button>
    <button onclick="setOnda('tri')"  id="w_tri">Triangular</button>
    <button onclick="setOnda('sq')"   id="w_sq">Cuadrada</button>
  </div>
  <div class="ctrl">
    <div class="lab"><span>Frecuencia</span><span id="fval">1000 Hz</span></div>
    <div class="genfila">
      <input type="range" id="fslider" min="1" max="10000" value="1000"
             oninput="genFreq(this.value,false)" onchange="genFreq(this.value,true)">
      <input type="number" id="fnum" min="1" max="10000" value="1000"
             onchange="genFreq(this.value,true)">
    </div>
  </div>
  <div class="ctrl">
    <div class="lab"><span>Amplitud</span><span id="aval">80 %</span></div>
    <input type="range" id="aslider" min="0" max="100" value="80"
           oninput="genAmp(this.value,false)" onchange="genAmp(this.value,true)">
  </div>
  <button onclick="genToggleOut()" id="bout" style="width:100%">Salida: OFF</button>
</div>

<div id="estado">conectado</div>
<script>
let modo = 1;
let sonando = false;
let audioCtx = null;
let oscAudio = null;

function initAudio(){
  if(!audioCtx){
    audioCtx = new (window.AudioContext || window.webkitAudioContext)();
  }
  if(audioCtx.state === 'suspended') audioCtx.resume();
}

function beepOn(){
  if(sonando || !audioCtx) return;
  oscAudio = audioCtx.createOscillator();
  const gain = audioCtx.createGain();
  oscAudio.type = 'square';
  oscAudio.frequency.value = 2000;
  gain.gain.value = 0.08;
  oscAudio.connect(gain).connect(audioCtx.destination);
  oscAudio.start();
  sonando = true;
}

function beepOff(){
  if(!sonando) return;
  oscAudio.stop();
  oscAudio.disconnect();
  sonando = false;
}

function setModo(m){
  initAudio();
  beepOff();
  modo = m;
  fetch('/modo?m='+m);
  for(let i=1;i<=6;i++){
    document.getElementById('b'+i).className = (i===m)?'activo':'';
  }
  document.getElementById('valor').textContent = '--';
}

let historial = [];

function actualizar(){
  fetch('/medir').then(r=>r.json()).then(d=>{
    // extrae el numero del texto (ej. "32.45 kOhm" -> 32.45)
    const num = parseFloat(d.v);
    const unidad = d.v.replace(/^[\d.-]+\s*/, '');

    if(!isNaN(num) && modo === 3){
      historial.push(num);
      if(historial.length > 5) historial.shift();   // ultimas 5 lecturas
      const prom = historial.reduce((a,b)=>a+b,0) / historial.length;
      document.getElementById('valor').textContent = prom.toFixed(2) + ' ' + unidad;
    } else {
      historial = [];
      document.getElementById('valor').textContent = d.v;
    }

    document.getElementById('estado').textContent = 'conectado';
    if(modo === 6 && d.v === 'CONTINUIDAD'){
      beepOn();
      if(navigator.vibrate) navigator.vibrate(50);
    } else {
      beepOff();
    }
  }).catch(()=>{
    document.getElementById('estado').textContent = 'sin conexion';
    beepOff();
  });
}

let oscOn = false;
let oscTimer = null;

function toggleOsc(){
  oscOn = !oscOn;
  document.getElementById('osc').style.display = oscOn ? 'block' : 'none';
  document.getElementById('bosc').className = oscOn ? 'activo' : '';
  if(oscOn){
    setModo(2);
    oscTimer = setInterval(dibujarOsc, 500);
  } else {
    clearInterval(oscTimer);
  }
}

function dibujarOsc(){
  fetch('/osc').then(r=>r.json()).then(d=>{
    const c = document.getElementById('osc');
    const g = c.getContext('2d');
    const W = c.width, H = c.height;
    g.clearRect(0,0,W,H);

    g.strokeStyle = '#e0e0e0'; g.lineWidth = 1;
    for(let i=1;i<5;i++){
      g.beginPath(); g.moveTo(0,H*i/5); g.lineTo(W,H*i/5); g.stroke();
      g.beginPath(); g.moveTo(W*i/5,0); g.lineTo(W*i/5,H); g.stroke();
    }

    const max = Math.max(1, ...d.d);
    g.strokeStyle = '#2b6cb0'; g.lineWidth = 2;
    g.beginPath();
    d.d.forEach((v,i)=>{
      const x = i * W / (d.d.length-1);
      const y = H - (v/max)*H*0.92 - 4;
      i===0 ? g.moveTo(x,y) : g.lineTo(x,y);
    });
    g.stroke();

    g.fillStyle = '#666'; g.font = '11px sans-serif';
    g.fillText(max.toFixed(1)+' V', 4, 12);
    const ms = (d.dt * d.d.length / 1000).toFixed(1);
    g.fillText(ms+' ms', W-46, H-5);
  });
}
function toggleMulti(){
  const p = document.getElementById('multipanel');
  const vis = p.style.display === 'block';
  p.style.display = vis ? 'none' : 'block';
  document.getElementById('bmulti').className = vis ? '' : 'activo';
}
// ---------------- Generador de funciones ----------------
let gen = {w:'sine', f:1000, a:80, on:false};

function toggleGen(){
  const p = document.getElementById('genpanel');
  const vis = p.style.display === 'block';
  p.style.display = vis ? 'none' : 'block';
  document.getElementById('bgen').className = vis ? '' : 'activo';
}

function genSend(){
  const a = gen.on ? gen.a : 0;               // OFF -> amplitud 0 (apaga la salida)
  fetch('/gen?w='+gen.w+'&f='+gen.f+'&a='+a).catch(()=>{});
}

function setOnda(w){
  gen.w = w;
  for(const id of ['sine','tri','sq'])
    document.getElementById('w_'+id).className = (id===w)?'activo':'';
  if(gen.on) genSend();
}

function fmtHz(f){ return f>=1000 ? (f/1000).toFixed(f%1000?1:0)+' kHz' : f+' Hz'; }

function genFreq(v, enviar){
  gen.f = Math.min(10000, Math.max(1, parseInt(v)||1));
  document.getElementById('fslider').value = gen.f;
  document.getElementById('fnum').value = gen.f;
  document.getElementById('fval').textContent = fmtHz(gen.f);
  if(enviar && gen.on) genSend();
}

function genAmp(v, enviar){
  gen.a = Math.min(100, Math.max(0, parseInt(v)||0));
  document.getElementById('aval').textContent = gen.a + ' %';
  if(enviar && gen.on) genSend();
}

function genToggleOut(){
  gen.on = !gen.on;
  const b = document.getElementById('bout');
  b.textContent = 'Salida: ' + (gen.on ? 'ON' : 'OFF');
  b.className = gen.on ? 'activo' : '';
  genSend();
}

setModo(1);
setInterval(actualizar, 1500);
</script></body></html>
)HTML";

static void handleRaiz()
{
  server.send_P(200, "text/html", PAGINA);
}

static void handleMedir()
{
  String v = leer_medicion();
  String json = "{\"v\":\"" + v + "\",\"m\":" + String(modo_activo) + "}";
  server.send(200, "application/json", json);
}

static void handleModo()
{
  if (server.hasArg("m"))
  {
    int m = server.arg("m").toInt();
    if (m >= 1 && m <= 6)
      cambiar_modo(m);
  }
  server.send(200, "text/plain", "ok");
}

static void handleOsc()
{
  server.send(200, "application/json", capturar_osciloscopio());
}

static void handleGen()
{
  String w = server.hasArg("w") ? server.arg("w") : "sine";
  float f = server.hasArg("f") ? server.arg("f").toFloat() : 1000.0f;
  int a = server.hasArg("a") ? server.arg("a").toInt() : 100;
  generador_aplicar(w, f, a);
  server.send(200, "application/json",
              "{\"w\":\"" + w + "\",\"f\":" + String(f, 1) + ",\"a\":" + String(a) + "}");
}

void servidor_setup()
{
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASS);
  Serial.print("[servidor] AP: ");
  Serial.println(AP_SSID);
  Serial.print("[servidor] IP: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", handleRaiz);
  server.on("/medir", handleMedir);
  server.on("/modo", handleModo);
  server.on("/osc", handleOsc);
  server.on("/gen", handleGen);
  server.begin();
  Serial.println("[servidor] Listo. Conectate y abre la IP.");
}

void servidor_loop()
{
  server.handleClient();
}