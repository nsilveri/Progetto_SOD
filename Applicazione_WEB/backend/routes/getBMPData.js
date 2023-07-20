const express = require('express');
const router = express.Router();
const getBMPDataController = require('../controllers/getBMPDataController');

router.post('/', getBMPDataController.handleGetBMPData);

module.exports = router;