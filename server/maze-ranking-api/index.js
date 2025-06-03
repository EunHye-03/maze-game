// index.js
require('dotenv').config();
const express = require('express');
const admin = require('firebase-admin');
const app = express();

app.use(express.json());

// Firebase Admin 초기화
admin.initializeApp({
  credential: admin.credential.cert({
    projectId: process.env.FIREBASE_PROJECT_ID,
    clientEmail: process.env.FIREBASE_CLIENT_EMAIL,
    // \n 포함된 PEM 문자열을 환경변수로 넣을 때 반드시 replace 처리
    privateKey: process.env.FIREBASE_PRIVATE_KEY.replace(/\\n/g, '\n')
  })
});

const db = admin.firestore();

// 📌 GET: 전체 점수 랭킹 조회
app.get('/scores', async (req, res) => {
  try {
    const snapshot = await db.collection('scores').orderBy('score', 'desc').limit(10).get();
    const scores = snapshot.docs.map(doc => ({ id: doc.id, ...doc.data() }));
    res.status(200).json(scores);
  } catch (err) {
    res.status(500).json({ error: 'Failed to fetch scores' });
  }
});

// 📌 POST: 새로운 점수 등록
app.post('/scores', async (req, res) => {
  const { username, score, level, lifes } = req.body;
  if (!username || score == null) {
    return res.status(400).json({ error: 'Missing required fields' });
  }

  try {
    const ref = await db.collection('scores').add({
      level: level || 1,
      score,
      timestamp: admin.firestore.FieldValue.serverTimestamp(),
      username
    });
    res.status(201).json({ id: ref.id, message: 'Score added' });
  } catch (err) {
    res.status(500).json({ error: 'Failed to add score' });
  }
});

// 서버 시작
const PORT = process.env.PORT || 3000;
app.listen(PORT, () => {
  console.log(`Server running on http://localhost:${PORT}`);
});
