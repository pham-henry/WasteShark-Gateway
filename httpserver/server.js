import express from 'express';
import axios from "axios";
import dotenv from "dotenv";

dotenv.config();

const app = express();
const PORT = process.env.PORT || 8080;
const GATEWAY_URL = process.env.GATEWAY_URL;

app.use(express.json());

console.log("[BACKEND] Starting server...");
console.log("[BACKEND] Gateway URL:", GATEWAY_URL);

// Helper to forward commands to the C gateway
async function forwardCommand(action) {
  try {
    console.log(`[BACKEND] Forwarding '${action}' to gateway...`);
    const res = await axios.post(GATEWAY_URL, { action });
    return { ok: true, status: res.status, data: res.data };
  } catch (err) {
    return {
      ok: false,
      status: err.response?.status || 500,
      error: err.response?.data || err.message,
    };
  }
}

// const robotRoutes = require("./routes/robots");

// // mount routes
// app.use("/robots", robotRoutes);

// app.get('/', (req, res) => {
//   res.send('Http Server Test');
// });

// ---- Robot Commands ----

// Health check
app.get("/health", (req, res) => {
  res.json({ status: "ok", gateway: GATEWAY_URL });
});

// Request robot to start
app.post("/api/start", async (req, res) => {
  const result = await forwardCommand("start");
  res.status(result.ok ? 200 : result.status).json(result);
});

// Request robot to stop
app.post("/api/stop", async (req, res) => {
  const result = await forwardCommand("stop");
  res.status(result.ok ? 200 : result.status).json(result);
});

// Gateway -> Backend telemetry endpoint
app.post("/api/telemetry", (req, res) => {
  console.log("[BACKEND] Telemetry:", req.body);
  res.json({ success: true });
});

// logging
app.listen(PORT, () => {
  console.log(`Server running on port http://localhost:${PORT}`);
});
