const express = require('express');
const router = express.Router();
const getBHDataController = require('../controllers/getBHDataController');

router.post('/', getBHDataController.handleGetBHData);

module.exports = router;