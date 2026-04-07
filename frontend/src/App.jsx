import { startTransition, useDeferredValue, useEffect, useEffectEvent, useRef, useState } from 'react';
import {
  appendHistory,
  buildSeedHistory,
  createDemoTelemetry,
  createTelemetrySnapshot,
  DEFAULT_SAMPLE_INTERVAL_MS,
  DEVICE_NAME,
  parseTelemetryLine,
} from './lib/telemetry';

const CONNECTION_STATES = {
  demo: 'Demo Stream',
  idle: 'Ready for Serial',
  connecting: 'Authorizing Port',
  streaming: 'Live Serial Stream',
  unsupported: 'Browser Unsupported',
  error: 'Connection Error',
};

export default function App() {
  const [mode, setMode] = useState('demo');
  const [telemetry, setTelemetry] = useState(() => createTelemetrySnapshot());
  const [history, setHistory] = useState(() => buildSeedHistory());
  const [rawMessage, setRawMessage] = useState(
    'WYP_TELEMETRY {"deviceName":"POstureTracker","sampleIntervalMs":1000,"calibrationSamples":200,"thresholdDegrees":10,"accel":{"x":0.01,"y":0.12,"z":0.98},"gyro":{"x":0.2,"y":-0.1,"z":0.04},"temp":36.4,"currentPitch":2.6,"baselinePitch":1.8,"pitchError":0.8,"currentRoll":-1.2,"baselineRoll":-1.6,"rollError":0.4,"postureGood":true}',
  );
  const [connection, setConnection] = useState({
    state: 'demo',
    message: 'Previewing the payload structure with a simulated live feed.',
    lastSeen: Date.now(),
    deviceName: DEVICE_NAME,
  });
  const [serialError, setSerialError] = useState('');

  const deferredHistory = useDeferredValue(history);
  const telemetryRef = useRef(telemetry);
  const demoStepRef = useRef(history.length);
  const serialPortRef = useRef(null);
  const readerRef = useRef(null);
  const readableClosedRef = useRef(null);
  const disconnectRequestedRef = useRef(false);

  useEffect(() => {
    telemetryRef.current = telemetry;
  }, [telemetry]);

  const commitTelemetry = useEffectEvent((nextTelemetry, rawLine = '') => {
    startTransition(() => {
      setTelemetry(nextTelemetry);
      setHistory((currentHistory) => appendHistory(currentHistory, nextTelemetry));
    });

    if (rawLine) {
      setRawMessage(rawLine);
    }

    setSerialError('');
    setConnection((current) => ({
      ...current,
      state: nextTelemetry.source === 'serial' ? 'streaming' : 'demo',
      message:
        nextTelemetry.source === 'serial'
          ? 'Reading structured telemetry from the ESP32 serial console.'
          : 'Previewing the payload structure with a simulated live feed.',
      lastSeen: nextTelemetry.receivedAt,
      deviceName: nextTelemetry.deviceName,
    }));
  });

  useEffect(() => {
    if (mode !== 'demo') {
      return undefined;
    }

    setConnection({
      state: 'demo',
      message: 'Previewing the payload structure with a simulated live feed.',
      lastSeen: Date.now(),
      deviceName: DEVICE_NAME,
    });

    const intervalId = window.setInterval(() => {
      demoStepRef.current += 1;
      commitTelemetry(createDemoTelemetry(demoStepRef.current));
    }, DEFAULT_SAMPLE_INTERVAL_MS);

    return () => window.clearInterval(intervalId);
  }, [mode, commitTelemetry]);

  useEffect(() => () => {
    void releaseSerialResources();
  }, []);

  async function handleModeChange(nextMode) {
    if (nextMode === mode) {
      return;
    }

    if (nextMode === 'demo') {
      disconnectRequestedRef.current = true;
      await releaseSerialResources();
      setSerialError('');
      setMode('demo');
      return;
    }

    setMode('serial');
    setConnection({
      state: 'idle',
      message: 'Use a Chromium browser, then connect to the ESP32 USB serial port.',
      lastSeen: telemetryRef.current.receivedAt,
      deviceName: telemetryRef.current.deviceName,
    });
  }

  async function connectSerial() {
    if (serialPortRef.current) {
      return;
    }

    if (!('serial' in navigator)) {
      setConnection({
        state: 'unsupported',
        message: 'Web Serial is available in Chromium-based browsers over a secure origin.',
        lastSeen: telemetryRef.current.receivedAt,
        deviceName: telemetryRef.current.deviceName,
      });
      return;
    }

    disconnectRequestedRef.current = false;
    setConnection({
      state: 'connecting',
      message: 'Approve the ESP32 serial port in the browser prompt to begin streaming.',
      lastSeen: telemetryRef.current.receivedAt,
      deviceName: telemetryRef.current.deviceName,
    });

    let nextErrorMessage = '';

    try {
      const port = await navigator.serial.requestPort();
      await port.open({ baudRate: 115200 });

      serialPortRef.current = port;

      const decoder = new TextDecoderStream();
      readableClosedRef.current = port.readable.pipeTo(decoder.writable);
      const lineReader = decoder.readable.pipeThrough(createLineBreakTransformer()).getReader();
      readerRef.current = lineReader;

      setConnection({
        state: 'streaming',
        message: 'Waiting for `WYP_TELEMETRY` packets from the board.',
        lastSeen: telemetryRef.current.receivedAt,
        deviceName: telemetryRef.current.deviceName,
      });

      while (true) {
        const { value, done } = await lineReader.read();
        if (done) {
          break;
        }

        const nextLine = value?.trim();
        if (!nextLine) {
          continue;
        }

        const parsedTelemetry = parseTelemetryLine(nextLine, telemetryRef.current);
        if (parsedTelemetry) {
          commitTelemetry(parsedTelemetry, nextLine);
        } else {
          setRawMessage(nextLine);
        }
      }
    } catch (error) {
      if (!disconnectRequestedRef.current) {
        nextErrorMessage = error instanceof Error ? error.message : 'Unable to open the serial device.';
        setSerialError(nextErrorMessage);
        setConnection({
          state: 'error',
          message: nextErrorMessage,
          lastSeen: telemetryRef.current.receivedAt,
          deviceName: telemetryRef.current.deviceName,
        });
      }
    } finally {
      await releaseSerialResources();

      if (!disconnectRequestedRef.current && mode === 'serial') {
        setConnection((current) => ({
          ...current,
          state: nextErrorMessage ? 'error' : 'idle',
          message: nextErrorMessage || 'The serial stream ended. Reconnect to resume live telemetry.',
        }));
      }
    }
  }

  async function disconnectSerial() {
    disconnectRequestedRef.current = true;
    await releaseSerialResources();
    setConnection({
      state: 'idle',
      message: 'Serial stream disconnected. Switch back to demo or reconnect the device.',
      lastSeen: telemetryRef.current.receivedAt,
      deviceName: telemetryRef.current.deviceName,
    });
  }

  async function releaseSerialResources() {
    const reader = readerRef.current;
    readerRef.current = null;
    if (reader) {
      try {
        await reader.cancel();
      } catch {}

      try {
        reader.releaseLock();
      } catch {}
    }

    const readableClosed = readableClosedRef.current;
    readableClosedRef.current = null;
    if (readableClosed) {
      try {
        await readableClosed;
      } catch {}
    }

    const port = serialPortRef.current;
    serialPortRef.current = null;
    if (port) {
      try {
        await port.close();
      } catch {}
    }
  }

  const lastSeenLabel = formatTimeAgo(connection.lastSeen);
  const statusTone = telemetry.postureGood ? 'good' : 'bad';

  return (
    <div className="app-shell">
      <header className="hero-card">
        <div className="hero-copy">
          <span className="eyebrow">Wear Your Posture</span>
          <h1>Signal-rich React frontend for everything the device measures.</h1>
          <p>
            The dashboard surfaces posture status, tilt baselines, pitch and roll error, raw
            accelerometer and gyroscope axes, temperature, calibration settings, and the latest
            telemetry packet from the board.
          </p>
        </div>

        <div className="hero-actions">
          <div className="mode-switch" role="tablist" aria-label="Telemetry mode">
            <button
              type="button"
              className={mode === 'demo' ? 'mode-button is-active' : 'mode-button'}
              onClick={() => {
                void handleModeChange('demo');
              }}
            >
              Demo Stream
            </button>
            <button
              type="button"
              className={mode === 'serial' ? 'mode-button is-active' : 'mode-button'}
              onClick={() => {
                void handleModeChange('serial');
              }}
            >
              Live Serial
            </button>
          </div>

          <div className="hero-note">
            BLE is already advertising in firmware, but live sensor streaming still needs GATT
            characteristics. The serial view works with the current project immediately.
          </div>
        </div>
      </header>

      <main className="dashboard-grid">
        <section className="panel spotlight-panel">
          <div className="panel-head">
            <div>
              <span className={`status-pill ${statusTone}`}>
                {telemetry.postureGood ? 'Aligned' : 'Needs attention'}
              </span>
              <h2>Connection + posture overview</h2>
            </div>
            <div className="connection-meta">
              <span>{CONNECTION_STATES[connection.state]}</span>
              <span>{lastSeenLabel}</span>
            </div>
          </div>

          <div className="spotlight-layout">
            <div className="connection-card">
              <div className="device-name">{connection.deviceName}</div>
              <p>{connection.message}</p>

              <dl className="meta-list">
                <div>
                  <dt>Calibration</dt>
                  <dd>{telemetry.calibrationSamples} samples</dd>
                </div>
                <div>
                  <dt>Sample interval</dt>
                  <dd>{telemetry.sampleIntervalMs} ms</dd>
                </div>
                <div>
                  <dt>Threshold</dt>
                  <dd>{telemetry.thresholdDegrees.toFixed(1)}&deg; safe band</dd>
                </div>
              </dl>

              {mode === 'serial' ? (
                <div className="action-row">
                  <button type="button" className="action-button" onClick={() => void connectSerial()}>
                    Connect Device
                  </button>
                  <button
                    type="button"
                    className="ghost-button"
                    onClick={() => void disconnectSerial()}
                    disabled={!serialPortRef.current}
                  >
                    Disconnect
                  </button>
                </div>
              ) : (
                <div className="action-row">
                  <button
                    type="button"
                    className="action-button"
                    onClick={() => {
                      void handleModeChange('serial');
                    }}
                  >
                    Switch to Live Serial
                  </button>
                </div>
              )}

              {serialError ? <p className="inline-error">{serialError}</p> : null}
            </div>

            <TiltDial telemetry={telemetry} />
          </div>
        </section>

        <section className="panel status-panel">
          <div className="panel-head">
            <div>
              <span className="eyebrow">Posture Snapshot</span>
              <h2>At-a-glance posture metrics</h2>
            </div>
          </div>

          <div className="metric-grid">
            <MetricCard label="Current pitch" value={formatAngle(telemetry.currentPitch)} accent="coral" />
            <MetricCard label="Baseline pitch" value={formatAngle(telemetry.baselinePitch)} accent="sand" />
            <MetricCard label="Pitch error" value={formatAngle(telemetry.pitchError, true)} accent="teal" />
            <MetricCard label="Current roll" value={formatAngle(telemetry.currentRoll)} accent="coral" />
            <MetricCard label="Baseline roll" value={formatAngle(telemetry.baselineRoll)} accent="sand" />
            <MetricCard label="Roll error" value={formatAngle(telemetry.rollError, true)} accent="teal" />
          </div>
        </section>

        <section className="panel sensors-panel">
          <div className="panel-head">
            <div>
              <span className="eyebrow">Raw Device Data</span>
              <h2>Sensor axes and temperature</h2>
            </div>
          </div>

          <div className="sensor-layout">
            <AxisPanel title="Accelerometer" unit="g" values={telemetry.accel} range={1.25} />
            <AxisPanel title="Gyroscope" unit="deg/s" values={telemetry.gyro} range={3.5} />
            <div className="temperature-card">
              <span className="eyebrow">Board temperature</span>
              <strong>{telemetry.temp.toFixed(2)}&deg;C</strong>
              <p>The same value the firmware reads from the MPU6050 packet.</p>
            </div>
          </div>
        </section>

        <section className="panel chart-panel">
          <div className="panel-head">
            <div>
              <span className="eyebrow">Pitch Trend</span>
              <h2>Pitch versus calibrated baseline</h2>
            </div>
          </div>
          <TrendChart
            history={deferredHistory}
            label="Pitch"
            valueKey="currentPitch"
            baselineKey="baselinePitch"
            thresholdDegrees={telemetry.thresholdDegrees}
            accent="var(--coral)"
          />
        </section>

        <section className="panel chart-panel">
          <div className="panel-head">
            <div>
              <span className="eyebrow">Roll Trend</span>
              <h2>Roll versus calibrated baseline</h2>
            </div>
          </div>
          <TrendChart
            history={deferredHistory}
            label="Roll"
            valueKey="currentRoll"
            baselineKey="baselineRoll"
            thresholdDegrees={telemetry.thresholdDegrees}
            accent="var(--teal)"
          />
        </section>

        <section className="panel raw-panel">
          <div className="panel-head">
            <div>
              <span className="eyebrow">Telemetry Packet</span>
              <h2>Latest message from the device</h2>
            </div>
          </div>

          <pre className="raw-packet">{rawMessage}</pre>
          <p className="support-copy">
            The frontend parses the new `WYP_TELEMETRY` JSON packet and still falls back to the
            older human-readable pitch and roll log line if needed.
          </p>
        </section>
      </main>
    </div>
  );
}

function MetricCard({ label, value, accent }) {
  return (
    <article className={`metric-card ${accent}`}>
      <span>{label}</span>
      <strong>{value}</strong>
    </article>
  );
}

function AxisPanel({ title, unit, values, range }) {
  return (
    <article className="axis-panel">
      <div className="axis-panel-head">
        <span className="eyebrow">{unit}</span>
        <h3>{title}</h3>
      </div>

      <div className="axis-stack">
        {['x', 'y', 'z'].map((axis) => (
          <AxisRow key={axis} axis={axis} value={values[axis]} range={range} />
        ))}
      </div>
    </article>
  );
}

function AxisRow({ axis, value, range }) {
  const normalized = Math.min(Math.abs(value) / range, 1);

  return (
    <div className="axis-row">
      <span className="axis-label">{axis.toUpperCase()}</span>
      <div className="axis-track">
        <div
          className={value >= 0 ? 'axis-fill positive' : 'axis-fill negative'}
          style={{ width: `${normalized * 50}%` }}
        />
      </div>
      <span className="axis-value">{value.toFixed(3)}</span>
    </div>
  );
}

function TiltDial({ telemetry }) {
  const limit = telemetry.thresholdDegrees * 1.5;
  const x = clamp(telemetry.rollError / limit, -1, 1);
  const y = clamp(telemetry.pitchError / limit, -1, 1);
  const pointX = 110 + x * 48;
  const pointY = 110 - y * 48;

  return (
    <div className="tilt-card">
      <div className="tilt-meta">
        <span className="eyebrow">Tilt Dial</span>
        <p>Centered means your live posture matches the calibrated baseline.</p>
      </div>

      <svg viewBox="0 0 220 220" className="tilt-dial" role="img" aria-label="Pitch and roll dial">
        <circle cx="110" cy="110" r="88" className="dial-outer" />
        <circle cx="110" cy="110" r="58" className="dial-safe" />
        <line x1="22" y1="110" x2="198" y2="110" className="dial-axis" />
        <line x1="110" y1="22" x2="110" y2="198" className="dial-axis" />
        <circle
          cx={pointX}
          cy={pointY}
          r="12"
          className={telemetry.postureGood ? 'dial-point safe' : 'dial-point alert'}
        />
      </svg>
    </div>
  );
}

function TrendChart({ history, label, valueKey, baselineKey, thresholdDegrees, accent }) {
  const width = 580;
  const height = 220;
  const padding = 22;

  const values = history.map((entry) => entry[valueKey]);
  const baselines = history.map((entry) => entry[baselineKey]);
  const baseline = baselines.at(-1) ?? 0;
  const minValue = Math.min(...values, ...baselines, baseline - thresholdDegrees) - 2;
  const maxValue = Math.max(...values, ...baselines, baseline + thresholdDegrees) + 2;
  const chartRange = maxValue - minValue || 1;

  const points = history.map((entry, index) => {
    const value = entry[valueKey];
    const x = padding + (index / Math.max(history.length - 1, 1)) * (width - padding * 2);
    const y = height - padding - ((value - minValue) / chartRange) * (height - padding * 2);
    return `${index === 0 ? 'M' : 'L'} ${x} ${y}`;
  });

  const baselineY =
    height - padding - ((baseline - minValue) / chartRange) * (height - padding * 2);
  const upperY =
    height -
    padding -
    ((baseline + thresholdDegrees - minValue) / chartRange) * (height - padding * 2);
  const lowerY =
    height -
    padding -
    ((baseline - thresholdDegrees - minValue) / chartRange) * (height - padding * 2);

  return (
    <div className="chart-shell">
      <svg viewBox={`0 0 ${width} ${height}`} className="trend-chart" role="img" aria-label={`${label} history`}>
        <rect x="0" y="0" width={width} height={height} rx="24" className="chart-bg" />
        <line x1={padding} y1={baselineY} x2={width - padding} y2={baselineY} className="chart-baseline" />
        <line x1={padding} y1={upperY} x2={width - padding} y2={upperY} className="chart-boundary" />
        <line x1={padding} y1={lowerY} x2={width - padding} y2={lowerY} className="chart-boundary" />
        <path d={points.join(' ')} className="chart-line" style={{ stroke: accent }} />
      </svg>

      <div className="chart-legend">
        <span>{label} live line</span>
        <span>Safe band: +/-{thresholdDegrees.toFixed(1)}&deg;</span>
        <span>Baseline: {formatAngle(baseline)}</span>
      </div>
    </div>
  );
}

function createLineBreakTransformer() {
  let chunkBuffer = '';

  return new TransformStream({
    transform(chunk, controller) {
      chunkBuffer += chunk;
      const lines = chunkBuffer.split(/\r?\n/);
      chunkBuffer = lines.pop() ?? '';
      lines.forEach((line) => controller.enqueue(line));
    },
    flush(controller) {
      if (chunkBuffer) {
        controller.enqueue(chunkBuffer);
      }
    },
  });
}

function formatAngle(value, signed = false) {
  const prefix = signed && value > 0 ? '+' : '';
  return `${prefix}${value.toFixed(1)} deg`;
}

function formatTimeAgo(timestamp) {
  const seconds = Math.max(0, Math.round((Date.now() - timestamp) / 1000));

  if (seconds < 2) {
    return 'Just now';
  }

  if (seconds < 60) {
    return `${seconds}s ago`;
  }

  const minutes = Math.round(seconds / 60);
  return `${minutes}m ago`;
}

function clamp(value, min, max) {
  return Math.min(Math.max(value, min), max);
}
