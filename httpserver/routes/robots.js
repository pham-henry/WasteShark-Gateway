const express = require("express");
const router = express.Router();

router.post("/start", (req, res) => {
  // logic to start robot
  res.json({ status: "robot started" });
});

router.post("/stop", (req, res) => {
  // logic to stop robot
  res.json({ status: "robot stopped" });
});

module.exports = router;
