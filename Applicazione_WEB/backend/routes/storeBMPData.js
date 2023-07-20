const express = require('express');
const router = express.Router();
const storeBMPDataController = require('../controllers/storeBMPDataController');

router.post('/', storeBMPDataController.handleStoreBMPData);

module.exports = router;