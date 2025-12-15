const express = require("express");
const app = express();

app.use(express.json());

const robotRoutes = require("./routes/robots");

// mount routes
app.use("/robots", robotRoutes);

app.get('/', (req, res) => {
  res.send('Http Server Test');
});

app.listen(3000, () => {
  console.log("Server running on port 3000");
});
