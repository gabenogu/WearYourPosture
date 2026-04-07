export const DEVICE_NAME = 'POstureTracker';
export const DEFAULT_THRESHOLD_DEGREES = 10;
export const DEFAULT_SAMPLE_INTERVAL_MS = 1000;
export const DEFAULT_CALIBRATION_SAMPLES = 200;
export const HISTORY_LIMIT = 30;

const DEFAULT_SNAPSHOT = {
  deviceName: DEVICE_NAME,
  sampleIntervalMs: DEFAULT_SAMPLE_INTERVAL_MS,
  calibrationSamples: DEFAULT_CALIBRATION_SAMPLES,
  thresholdDegrees: DEFAULT_THRESHOLD_DEGREES,
  accel: { x: 0.01, y: 0.12, z: 0.98 },
  gyro: { x: 0.2, y: -0.1, z: 0.04 },
  temp: 36.4,
  currentPitch: 2.6,
  baselinePitch: 1.8,
  pitchError: 0.8,
  currentRoll: -1.2,
  baselineRoll: -1.6,
  rollError: 0.4,
  postureGood: true,
  source: 'demo',
  receivedAt: Date.now(),
};

export function createTelemetrySnapshot(overrides = {}) {
  return normalizeTelemetryPayload({ ...DEFAULT_SNAPSHOT, ...overrides }, DEFAULT_SNAPSHOT, overrides.source ?? 'demo');
}

export function createDemoTelemetry(step = 0) {
  const baselinePitch = 1.8;
  const baselineRoll = -1.6;
  const thresholdDegrees = DEFAULT_THRESHOLD_DEGREES;

  const sway = Math.sin(step / 3.5);
  const counterSway = Math.cos(step / 4.2);
  const slump = Math.max(0, Math.sin((step - 3) / 2.4));

  const currentPitch = baselinePitch + sway * 4.6 + slump * 8.4;
  const currentRoll = baselineRoll + counterSway * 3.4 - Math.max(0, Math.cos((step + 2) / 3)) * 3.1;
  const pitchError = currentPitch - baselinePitch;
  const rollError = currentRoll - baselineRoll;

  return createTelemetrySnapshot({
    currentPitch: round(currentPitch),
    baselinePitch,
    pitchError: round(pitchError),
    currentRoll: round(currentRoll),
    baselineRoll,
    rollError: round(rollError),
    thresholdDegrees,
    accel: {
      x: round(Math.sin(step / 5) * 0.22, 3),
      y: round(0.12 + Math.cos(step / 4.7) * 0.18, 3),
      z: round(0.95 - Math.abs(Math.sin(step / 5.4)) * 0.12, 3),
    },
    gyro: {
      x: round(Math.cos(step / 2.8) * 2.2, 3),
      y: round(Math.sin(step / 2.4) * 1.7, 3),
      z: round(Math.cos(step / 3.2) * 1.3, 3),
    },
    temp: round(36.4 + Math.sin(step / 7) * 0.45, 2),
    postureGood:
      Math.abs(pitchError) <= thresholdDegrees && Math.abs(rollError) <= thresholdDegrees,
    source: 'demo',
    receivedAt: Date.now(),
  });
}

export function buildSeedHistory(count = 18) {
  return Array.from({ length: count }, (_, index) => createDemoTelemetry(index - count + 1));
}

export function appendHistory(history, telemetry) {
  return [...history, telemetry].slice(-HISTORY_LIMIT);
}

export function parseTelemetryLine(line, fallback = DEFAULT_SNAPSHOT) {
  const jsonMatch = line.match(/^WYP_TELEMETRY\s+(\{.+\})$/);
  if (jsonMatch) {
    try {
      return normalizeTelemetryPayload(JSON.parse(jsonMatch[1]), fallback, 'serial');
    } catch {
      return null;
    }
  }

  const legacyMatch = line.match(
    /Current Pitch:\s*([-+]?\d*\.?\d+),\s*Baseline Pitch:\s*([-+]?\d*\.?\d+),\s*Pitch Error:\s*([-+]?\d*\.?\d+),\s*Current Roll:\s*([-+]?\d*\.?\d+),\s*Baseline Roll:\s*([-+]?\d*\.?\d+),\s*Roll Error:\s*([-+]?\d*\.?\d+),\s*Posture Good:\s*(Yes|No)/i,
  );

  if (!legacyMatch) {
    return null;
  }

  const [
    ,
    currentPitch,
    baselinePitch,
    pitchError,
    currentRoll,
    baselineRoll,
    rollError,
    postureGood,
  ] = legacyMatch;

  return normalizeTelemetryPayload(
    {
      ...fallback,
      currentPitch: Number(currentPitch),
      baselinePitch: Number(baselinePitch),
      pitchError: Number(pitchError),
      currentRoll: Number(currentRoll),
      baselineRoll: Number(baselineRoll),
      rollError: Number(rollError),
      postureGood: postureGood.toLowerCase() === 'yes',
    },
    fallback,
    'serial',
  );
}

function normalizeTelemetryPayload(payload, fallback, source) {
  const thresholdDegrees = toNumber(payload.thresholdDegrees, fallback.thresholdDegrees, DEFAULT_THRESHOLD_DEGREES);
  const baselinePitch = toNumber(payload.baselinePitch, fallback.baselinePitch, DEFAULT_SNAPSHOT.baselinePitch);
  const baselineRoll = toNumber(payload.baselineRoll, fallback.baselineRoll, DEFAULT_SNAPSHOT.baselineRoll);
  const currentPitch = toNumber(payload.currentPitch, fallback.currentPitch, DEFAULT_SNAPSHOT.currentPitch);
  const currentRoll = toNumber(payload.currentRoll, fallback.currentRoll, DEFAULT_SNAPSHOT.currentRoll);

  return {
    deviceName: payload.deviceName ?? fallback.deviceName ?? DEVICE_NAME,
    sampleIntervalMs: toNumber(
      payload.sampleIntervalMs,
      fallback.sampleIntervalMs,
      DEFAULT_SAMPLE_INTERVAL_MS,
    ),
    calibrationSamples: toNumber(
      payload.calibrationSamples,
      fallback.calibrationSamples,
      DEFAULT_CALIBRATION_SAMPLES,
    ),
    thresholdDegrees,
    accel: {
      x: toNumber(payload.accel?.x, fallback.accel?.x, DEFAULT_SNAPSHOT.accel.x),
      y: toNumber(payload.accel?.y, fallback.accel?.y, DEFAULT_SNAPSHOT.accel.y),
      z: toNumber(payload.accel?.z, fallback.accel?.z, DEFAULT_SNAPSHOT.accel.z),
    },
    gyro: {
      x: toNumber(payload.gyro?.x, fallback.gyro?.x, DEFAULT_SNAPSHOT.gyro.x),
      y: toNumber(payload.gyro?.y, fallback.gyro?.y, DEFAULT_SNAPSHOT.gyro.y),
      z: toNumber(payload.gyro?.z, fallback.gyro?.z, DEFAULT_SNAPSHOT.gyro.z),
    },
    temp: toNumber(payload.temp, fallback.temp, DEFAULT_SNAPSHOT.temp),
    currentPitch,
    baselinePitch,
    pitchError: toNumber(payload.pitchError, currentPitch - baselinePitch, fallback.pitchError),
    currentRoll,
    baselineRoll,
    rollError: toNumber(payload.rollError, currentRoll - baselineRoll, fallback.rollError),
    postureGood:
      payload.postureGood === true ||
      payload.postureGood === 'true' ||
      payload.postureGood === 1 ||
      payload.postureGood === '1',
    source,
    receivedAt: Date.now(),
  };
}

function toNumber(value, fallback, finalFallback) {
  const parsed = Number(value);
  if (Number.isFinite(parsed)) {
    return parsed;
  }

  if (Number.isFinite(fallback)) {
    return fallback;
  }

  return finalFallback;
}

function round(value, digits = 2) {
  const precision = 10 ** digits;
  return Math.round(value * precision) / precision;
}
