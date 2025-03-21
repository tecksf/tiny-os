# 初始化中断处理函数的入口地址数组
# 共256种中断，入口地址保存在__vectors数组中
# 0~32中断都会压入一个错误码，但这里都是可屏蔽的外设中断，没有错误码;为了保持统一，压入一个0
# 每个中断都压入自己的中断号，然后跳转到__all_traps执行
.text
.globl __all_traps
.globl vector0
vector0:
  pushl $0
  pushl $0
  jmp __all_traps
.globl vector1
vector1:
  pushl $0
  pushl $1
  jmp __all_traps
.globl vector2
vector2:
  pushl $0
  pushl $2
  jmp __all_traps
.globl vector3
vector3:
  pushl $0
  pushl $3
  jmp __all_traps
.globl vector4
vector4:
  pushl $0
  pushl $4
  jmp __all_traps
.globl vector5
vector5:
  pushl $0
  pushl $5
  jmp __all_traps
.globl vector6
vector6:
  pushl $0
  pushl $6
  jmp __all_traps
.globl vector7
vector7:
  pushl $0
  pushl $7
  jmp __all_traps
.globl vector8
vector8:
  pushl $8
  jmp __all_traps
.globl vector9
vector9:
  pushl $0
  pushl $9
  jmp __all_traps
.globl vector10
vector10:
  pushl $10
  jmp __all_traps
.globl vector11
vector11:
  pushl $11
  jmp __all_traps
.globl vector12
vector12:
  pushl $12
  jmp __all_traps
.globl vector13
vector13:
  pushl $13
  jmp __all_traps
.globl vector14
vector14:
  pushl $14
  jmp __all_traps
.globl vector15
vector15:
  pushl $0
  pushl $15
  jmp __all_traps
.globl vector16
vector16:
  pushl $0
  pushl $16
  jmp __all_traps
.globl vector17
vector17:
  pushl $17
  jmp __all_traps
.globl vector18
vector18:
  pushl $0
  pushl $18
  jmp __all_traps
.globl vector19
vector19:
  pushl $0
  pushl $19
  jmp __all_traps
.globl vector20
vector20:
  pushl $0
  pushl $20
  jmp __all_traps
.globl vector21
vector21:
  pushl $0
  pushl $21
  jmp __all_traps
.globl vector22
vector22:
  pushl $0
  pushl $22
  jmp __all_traps
.globl vector23
vector23:
  pushl $0
  pushl $23
  jmp __all_traps
.globl vector24
vector24:
  pushl $0
  pushl $24
  jmp __all_traps
.globl vector25
vector25:
  pushl $0
  pushl $25
  jmp __all_traps
.globl vector26
vector26:
  pushl $0
  pushl $26
  jmp __all_traps
.globl vector27
vector27:
  pushl $0
  pushl $27
  jmp __all_traps
.globl vector28
vector28:
  pushl $0
  pushl $28
  jmp __all_traps
.globl vector29
vector29:
  pushl $0
  pushl $29
  jmp __all_traps
.globl vector30
vector30:
  pushl $0
  pushl $30
  jmp __all_traps
.globl vector31
vector31:
  pushl $0
  pushl $31
  jmp __all_traps
.globl vector32
vector32:
  pushl $0
  pushl $32
  jmp __all_traps
.globl vector33
vector33:
  pushl $0
  pushl $33
  jmp __all_traps
.globl vector34
vector34:
  pushl $0
  pushl $34
  jmp __all_traps
.globl vector35
vector35:
  pushl $0
  pushl $35
  jmp __all_traps
.globl vector36
vector36:
  pushl $0
  pushl $36
  jmp __all_traps
.globl vector37
vector37:
  pushl $0
  pushl $37
  jmp __all_traps
.globl vector38
vector38:
  pushl $0
  pushl $38
  jmp __all_traps
.globl vector39
vector39:
  pushl $0
  pushl $39
  jmp __all_traps
.globl vector40
vector40:
  pushl $0
  pushl $40
  jmp __all_traps
.globl vector41
vector41:
  pushl $0
  pushl $41
  jmp __all_traps
.globl vector42
vector42:
  pushl $0
  pushl $42
  jmp __all_traps
.globl vector43
vector43:
  pushl $0
  pushl $43
  jmp __all_traps
.globl vector44
vector44:
  pushl $0
  pushl $44
  jmp __all_traps
.globl vector45
vector45:
  pushl $0
  pushl $45
  jmp __all_traps
.globl vector46
vector46:
  pushl $0
  pushl $46
  jmp __all_traps
.globl vector47
vector47:
  pushl $0
  pushl $47
  jmp __all_traps
.globl vector48
vector48:
  pushl $0
  pushl $48
  jmp __all_traps
.globl vector49
vector49:
  pushl $0
  pushl $49
  jmp __all_traps
.globl vector50
vector50:
  pushl $0
  pushl $50
  jmp __all_traps
.globl vector51
vector51:
  pushl $0
  pushl $51
  jmp __all_traps
.globl vector52
vector52:
  pushl $0
  pushl $52
  jmp __all_traps
.globl vector53
vector53:
  pushl $0
  pushl $53
  jmp __all_traps
.globl vector54
vector54:
  pushl $0
  pushl $54
  jmp __all_traps
.globl vector55
vector55:
  pushl $0
  pushl $55
  jmp __all_traps
.globl vector56
vector56:
  pushl $0
  pushl $56
  jmp __all_traps
.globl vector57
vector57:
  pushl $0
  pushl $57
  jmp __all_traps
.globl vector58
vector58:
  pushl $0
  pushl $58
  jmp __all_traps
.globl vector59
vector59:
  pushl $0
  pushl $59
  jmp __all_traps
.globl vector60
vector60:
  pushl $0
  pushl $60
  jmp __all_traps
.globl vector61
vector61:
  pushl $0
  pushl $61
  jmp __all_traps
.globl vector62
vector62:
  pushl $0
  pushl $62
  jmp __all_traps
.globl vector63
vector63:
  pushl $0
  pushl $63
  jmp __all_traps
.globl vector64
vector64:
  pushl $0
  pushl $64
  jmp __all_traps
.globl vector65
vector65:
  pushl $0
  pushl $65
  jmp __all_traps
.globl vector66
vector66:
  pushl $0
  pushl $66
  jmp __all_traps
.globl vector67
vector67:
  pushl $0
  pushl $67
  jmp __all_traps
.globl vector68
vector68:
  pushl $0
  pushl $68
  jmp __all_traps
.globl vector69
vector69:
  pushl $0
  pushl $69
  jmp __all_traps
.globl vector70
vector70:
  pushl $0
  pushl $70
  jmp __all_traps
.globl vector71
vector71:
  pushl $0
  pushl $71
  jmp __all_traps
.globl vector72
vector72:
  pushl $0
  pushl $72
  jmp __all_traps
.globl vector73
vector73:
  pushl $0
  pushl $73
  jmp __all_traps
.globl vector74
vector74:
  pushl $0
  pushl $74
  jmp __all_traps
.globl vector75
vector75:
  pushl $0
  pushl $75
  jmp __all_traps
.globl vector76
vector76:
  pushl $0
  pushl $76
  jmp __all_traps
.globl vector77
vector77:
  pushl $0
  pushl $77
  jmp __all_traps
.globl vector78
vector78:
  pushl $0
  pushl $78
  jmp __all_traps
.globl vector79
vector79:
  pushl $0
  pushl $79
  jmp __all_traps
.globl vector80
vector80:
  pushl $0
  pushl $80
  jmp __all_traps
.globl vector81
vector81:
  pushl $0
  pushl $81
  jmp __all_traps
.globl vector82
vector82:
  pushl $0
  pushl $82
  jmp __all_traps
.globl vector83
vector83:
  pushl $0
  pushl $83
  jmp __all_traps
.globl vector84
vector84:
  pushl $0
  pushl $84
  jmp __all_traps
.globl vector85
vector85:
  pushl $0
  pushl $85
  jmp __all_traps
.globl vector86
vector86:
  pushl $0
  pushl $86
  jmp __all_traps
.globl vector87
vector87:
  pushl $0
  pushl $87
  jmp __all_traps
.globl vector88
vector88:
  pushl $0
  pushl $88
  jmp __all_traps
.globl vector89
vector89:
  pushl $0
  pushl $89
  jmp __all_traps
.globl vector90
vector90:
  pushl $0
  pushl $90
  jmp __all_traps
.globl vector91
vector91:
  pushl $0
  pushl $91
  jmp __all_traps
.globl vector92
vector92:
  pushl $0
  pushl $92
  jmp __all_traps
.globl vector93
vector93:
  pushl $0
  pushl $93
  jmp __all_traps
.globl vector94
vector94:
  pushl $0
  pushl $94
  jmp __all_traps
.globl vector95
vector95:
  pushl $0
  pushl $95
  jmp __all_traps
.globl vector96
vector96:
  pushl $0
  pushl $96
  jmp __all_traps
.globl vector97
vector97:
  pushl $0
  pushl $97
  jmp __all_traps
.globl vector98
vector98:
  pushl $0
  pushl $98
  jmp __all_traps
.globl vector99
vector99:
  pushl $0
  pushl $99
  jmp __all_traps
.globl vector100
vector100:
  pushl $0
  pushl $100
  jmp __all_traps
.globl vector101
vector101:
  pushl $0
  pushl $101
  jmp __all_traps
.globl vector102
vector102:
  pushl $0
  pushl $102
  jmp __all_traps
.globl vector103
vector103:
  pushl $0
  pushl $103
  jmp __all_traps
.globl vector104
vector104:
  pushl $0
  pushl $104
  jmp __all_traps
.globl vector105
vector105:
  pushl $0
  pushl $105
  jmp __all_traps
.globl vector106
vector106:
  pushl $0
  pushl $106
  jmp __all_traps
.globl vector107
vector107:
  pushl $0
  pushl $107
  jmp __all_traps
.globl vector108
vector108:
  pushl $0
  pushl $108
  jmp __all_traps
.globl vector109
vector109:
  pushl $0
  pushl $109
  jmp __all_traps
.globl vector110
vector110:
  pushl $0
  pushl $110
  jmp __all_traps
.globl vector111
vector111:
  pushl $0
  pushl $111
  jmp __all_traps
.globl vector112
vector112:
  pushl $0
  pushl $112
  jmp __all_traps
.globl vector113
vector113:
  pushl $0
  pushl $113
  jmp __all_traps
.globl vector114
vector114:
  pushl $0
  pushl $114
  jmp __all_traps
.globl vector115
vector115:
  pushl $0
  pushl $115
  jmp __all_traps
.globl vector116
vector116:
  pushl $0
  pushl $116
  jmp __all_traps
.globl vector117
vector117:
  pushl $0
  pushl $117
  jmp __all_traps
.globl vector118
vector118:
  pushl $0
  pushl $118
  jmp __all_traps
.globl vector119
vector119:
  pushl $0
  pushl $119
  jmp __all_traps
.globl vector120
vector120:
  pushl $0
  pushl $120
  jmp __all_traps
.globl vector121
vector121:
  pushl $0
  pushl $121
  jmp __all_traps
.globl vector122
vector122:
  pushl $0
  pushl $122
  jmp __all_traps
.globl vector123
vector123:
  pushl $0
  pushl $123
  jmp __all_traps
.globl vector124
vector124:
  pushl $0
  pushl $124
  jmp __all_traps
.globl vector125
vector125:
  pushl $0
  pushl $125
  jmp __all_traps
.globl vector126
vector126:
  pushl $0
  pushl $126
  jmp __all_traps
.globl vector127
vector127:
  pushl $0
  pushl $127
  jmp __all_traps
.globl vector128
vector128:
  pushl $0
  pushl $128
  jmp __all_traps
.globl vector129
vector129:
  pushl $0
  pushl $129
  jmp __all_traps
.globl vector130
vector130:
  pushl $0
  pushl $130
  jmp __all_traps
.globl vector131
vector131:
  pushl $0
  pushl $131
  jmp __all_traps
.globl vector132
vector132:
  pushl $0
  pushl $132
  jmp __all_traps
.globl vector133
vector133:
  pushl $0
  pushl $133
  jmp __all_traps
.globl vector134
vector134:
  pushl $0
  pushl $134
  jmp __all_traps
.globl vector135
vector135:
  pushl $0
  pushl $135
  jmp __all_traps
.globl vector136
vector136:
  pushl $0
  pushl $136
  jmp __all_traps
.globl vector137
vector137:
  pushl $0
  pushl $137
  jmp __all_traps
.globl vector138
vector138:
  pushl $0
  pushl $138
  jmp __all_traps
.globl vector139
vector139:
  pushl $0
  pushl $139
  jmp __all_traps
.globl vector140
vector140:
  pushl $0
  pushl $140
  jmp __all_traps
.globl vector141
vector141:
  pushl $0
  pushl $141
  jmp __all_traps
.globl vector142
vector142:
  pushl $0
  pushl $142
  jmp __all_traps
.globl vector143
vector143:
  pushl $0
  pushl $143
  jmp __all_traps
.globl vector144
vector144:
  pushl $0
  pushl $144
  jmp __all_traps
.globl vector145
vector145:
  pushl $0
  pushl $145
  jmp __all_traps
.globl vector146
vector146:
  pushl $0
  pushl $146
  jmp __all_traps
.globl vector147
vector147:
  pushl $0
  pushl $147
  jmp __all_traps
.globl vector148
vector148:
  pushl $0
  pushl $148
  jmp __all_traps
.globl vector149
vector149:
  pushl $0
  pushl $149
  jmp __all_traps
.globl vector150
vector150:
  pushl $0
  pushl $150
  jmp __all_traps
.globl vector151
vector151:
  pushl $0
  pushl $151
  jmp __all_traps
.globl vector152
vector152:
  pushl $0
  pushl $152
  jmp __all_traps
.globl vector153
vector153:
  pushl $0
  pushl $153
  jmp __all_traps
.globl vector154
vector154:
  pushl $0
  pushl $154
  jmp __all_traps
.globl vector155
vector155:
  pushl $0
  pushl $155
  jmp __all_traps
.globl vector156
vector156:
  pushl $0
  pushl $156
  jmp __all_traps
.globl vector157
vector157:
  pushl $0
  pushl $157
  jmp __all_traps
.globl vector158
vector158:
  pushl $0
  pushl $158
  jmp __all_traps
.globl vector159
vector159:
  pushl $0
  pushl $159
  jmp __all_traps
.globl vector160
vector160:
  pushl $0
  pushl $160
  jmp __all_traps
.globl vector161
vector161:
  pushl $0
  pushl $161
  jmp __all_traps
.globl vector162
vector162:
  pushl $0
  pushl $162
  jmp __all_traps
.globl vector163
vector163:
  pushl $0
  pushl $163
  jmp __all_traps
.globl vector164
vector164:
  pushl $0
  pushl $164
  jmp __all_traps
.globl vector165
vector165:
  pushl $0
  pushl $165
  jmp __all_traps
.globl vector166
vector166:
  pushl $0
  pushl $166
  jmp __all_traps
.globl vector167
vector167:
  pushl $0
  pushl $167
  jmp __all_traps
.globl vector168
vector168:
  pushl $0
  pushl $168
  jmp __all_traps
.globl vector169
vector169:
  pushl $0
  pushl $169
  jmp __all_traps
.globl vector170
vector170:
  pushl $0
  pushl $170
  jmp __all_traps
.globl vector171
vector171:
  pushl $0
  pushl $171
  jmp __all_traps
.globl vector172
vector172:
  pushl $0
  pushl $172
  jmp __all_traps
.globl vector173
vector173:
  pushl $0
  pushl $173
  jmp __all_traps
.globl vector174
vector174:
  pushl $0
  pushl $174
  jmp __all_traps
.globl vector175
vector175:
  pushl $0
  pushl $175
  jmp __all_traps
.globl vector176
vector176:
  pushl $0
  pushl $176
  jmp __all_traps
.globl vector177
vector177:
  pushl $0
  pushl $177
  jmp __all_traps
.globl vector178
vector178:
  pushl $0
  pushl $178
  jmp __all_traps
.globl vector179
vector179:
  pushl $0
  pushl $179
  jmp __all_traps
.globl vector180
vector180:
  pushl $0
  pushl $180
  jmp __all_traps
.globl vector181
vector181:
  pushl $0
  pushl $181
  jmp __all_traps
.globl vector182
vector182:
  pushl $0
  pushl $182
  jmp __all_traps
.globl vector183
vector183:
  pushl $0
  pushl $183
  jmp __all_traps
.globl vector184
vector184:
  pushl $0
  pushl $184
  jmp __all_traps
.globl vector185
vector185:
  pushl $0
  pushl $185
  jmp __all_traps
.globl vector186
vector186:
  pushl $0
  pushl $186
  jmp __all_traps
.globl vector187
vector187:
  pushl $0
  pushl $187
  jmp __all_traps
.globl vector188
vector188:
  pushl $0
  pushl $188
  jmp __all_traps
.globl vector189
vector189:
  pushl $0
  pushl $189
  jmp __all_traps
.globl vector190
vector190:
  pushl $0
  pushl $190
  jmp __all_traps
.globl vector191
vector191:
  pushl $0
  pushl $191
  jmp __all_traps
.globl vector192
vector192:
  pushl $0
  pushl $192
  jmp __all_traps
.globl vector193
vector193:
  pushl $0
  pushl $193
  jmp __all_traps
.globl vector194
vector194:
  pushl $0
  pushl $194
  jmp __all_traps
.globl vector195
vector195:
  pushl $0
  pushl $195
  jmp __all_traps
.globl vector196
vector196:
  pushl $0
  pushl $196
  jmp __all_traps
.globl vector197
vector197:
  pushl $0
  pushl $197
  jmp __all_traps
.globl vector198
vector198:
  pushl $0
  pushl $198
  jmp __all_traps
.globl vector199
vector199:
  pushl $0
  pushl $199
  jmp __all_traps
.globl vector200
vector200:
  pushl $0
  pushl $200
  jmp __all_traps
.globl vector201
vector201:
  pushl $0
  pushl $201
  jmp __all_traps
.globl vector202
vector202:
  pushl $0
  pushl $202
  jmp __all_traps
.globl vector203
vector203:
  pushl $0
  pushl $203
  jmp __all_traps
.globl vector204
vector204:
  pushl $0
  pushl $204
  jmp __all_traps
.globl vector205
vector205:
  pushl $0
  pushl $205
  jmp __all_traps
.globl vector206
vector206:
  pushl $0
  pushl $206
  jmp __all_traps
.globl vector207
vector207:
  pushl $0
  pushl $207
  jmp __all_traps
.globl vector208
vector208:
  pushl $0
  pushl $208
  jmp __all_traps
.globl vector209
vector209:
  pushl $0
  pushl $209
  jmp __all_traps
.globl vector210
vector210:
  pushl $0
  pushl $210
  jmp __all_traps
.globl vector211
vector211:
  pushl $0
  pushl $211
  jmp __all_traps
.globl vector212
vector212:
  pushl $0
  pushl $212
  jmp __all_traps
.globl vector213
vector213:
  pushl $0
  pushl $213
  jmp __all_traps
.globl vector214
vector214:
  pushl $0
  pushl $214
  jmp __all_traps
.globl vector215
vector215:
  pushl $0
  pushl $215
  jmp __all_traps
.globl vector216
vector216:
  pushl $0
  pushl $216
  jmp __all_traps
.globl vector217
vector217:
  pushl $0
  pushl $217
  jmp __all_traps
.globl vector218
vector218:
  pushl $0
  pushl $218
  jmp __all_traps
.globl vector219
vector219:
  pushl $0
  pushl $219
  jmp __all_traps
.globl vector220
vector220:
  pushl $0
  pushl $220
  jmp __all_traps
.globl vector221
vector221:
  pushl $0
  pushl $221
  jmp __all_traps
.globl vector222
vector222:
  pushl $0
  pushl $222
  jmp __all_traps
.globl vector223
vector223:
  pushl $0
  pushl $223
  jmp __all_traps
.globl vector224
vector224:
  pushl $0
  pushl $224
  jmp __all_traps
.globl vector225
vector225:
  pushl $0
  pushl $225
  jmp __all_traps
.globl vector226
vector226:
  pushl $0
  pushl $226
  jmp __all_traps
.globl vector227
vector227:
  pushl $0
  pushl $227
  jmp __all_traps
.globl vector228
vector228:
  pushl $0
  pushl $228
  jmp __all_traps
.globl vector229
vector229:
  pushl $0
  pushl $229
  jmp __all_traps
.globl vector230
vector230:
  pushl $0
  pushl $230
  jmp __all_traps
.globl vector231
vector231:
  pushl $0
  pushl $231
  jmp __all_traps
.globl vector232
vector232:
  pushl $0
  pushl $232
  jmp __all_traps
.globl vector233
vector233:
  pushl $0
  pushl $233
  jmp __all_traps
.globl vector234
vector234:
  pushl $0
  pushl $234
  jmp __all_traps
.globl vector235
vector235:
  pushl $0
  pushl $235
  jmp __all_traps
.globl vector236
vector236:
  pushl $0
  pushl $236
  jmp __all_traps
.globl vector237
vector237:
  pushl $0
  pushl $237
  jmp __all_traps
.globl vector238
vector238:
  pushl $0
  pushl $238
  jmp __all_traps
.globl vector239
vector239:
  pushl $0
  pushl $239
  jmp __all_traps
.globl vector240
vector240:
  pushl $0
  pushl $240
  jmp __all_traps
.globl vector241
vector241:
  pushl $0
  pushl $241
  jmp __all_traps
.globl vector242
vector242:
  pushl $0
  pushl $242
  jmp __all_traps
.globl vector243
vector243:
  pushl $0
  pushl $243
  jmp __all_traps
.globl vector244
vector244:
  pushl $0
  pushl $244
  jmp __all_traps
.globl vector245
vector245:
  pushl $0
  pushl $245
  jmp __all_traps
.globl vector246
vector246:
  pushl $0
  pushl $246
  jmp __all_traps
.globl vector247
vector247:
  pushl $0
  pushl $247
  jmp __all_traps
.globl vector248
vector248:
  pushl $0
  pushl $248
  jmp __all_traps
.globl vector249
vector249:
  pushl $0
  pushl $249
  jmp __all_traps
.globl vector250
vector250:
  pushl $0
  pushl $250
  jmp __all_traps
.globl vector251
vector251:
  pushl $0
  pushl $251
  jmp __all_traps
.globl vector252
vector252:
  pushl $0
  pushl $252
  jmp __all_traps
.globl vector253
vector253:
  pushl $0
  pushl $253
  jmp __all_traps
.globl vector254
vector254:
  pushl $0
  pushl $254
  jmp __all_traps
.globl vector255
vector255:
  pushl $0
  pushl $255
  jmp __all_traps

# vector table
.data
.globl __vectors
__vectors:
  .long vector0
  .long vector1
  .long vector2
  .long vector3
  .long vector4
  .long vector5
  .long vector6
  .long vector7
  .long vector8
  .long vector9
  .long vector10
  .long vector11
  .long vector12
  .long vector13
  .long vector14
  .long vector15
  .long vector16
  .long vector17
  .long vector18
  .long vector19
  .long vector20
  .long vector21
  .long vector22
  .long vector23
  .long vector24
  .long vector25
  .long vector26
  .long vector27
  .long vector28
  .long vector29
  .long vector30
  .long vector31
  .long vector32
  .long vector33
  .long vector34
  .long vector35
  .long vector36
  .long vector37
  .long vector38
  .long vector39
  .long vector40
  .long vector41
  .long vector42
  .long vector43
  .long vector44
  .long vector45
  .long vector46
  .long vector47
  .long vector48
  .long vector49
  .long vector50
  .long vector51
  .long vector52
  .long vector53
  .long vector54
  .long vector55
  .long vector56
  .long vector57
  .long vector58
  .long vector59
  .long vector60
  .long vector61
  .long vector62
  .long vector63
  .long vector64
  .long vector65
  .long vector66
  .long vector67
  .long vector68
  .long vector69
  .long vector70
  .long vector71
  .long vector72
  .long vector73
  .long vector74
  .long vector75
  .long vector76
  .long vector77
  .long vector78
  .long vector79
  .long vector80
  .long vector81
  .long vector82
  .long vector83
  .long vector84
  .long vector85
  .long vector86
  .long vector87
  .long vector88
  .long vector89
  .long vector90
  .long vector91
  .long vector92
  .long vector93
  .long vector94
  .long vector95
  .long vector96
  .long vector97
  .long vector98
  .long vector99
  .long vector100
  .long vector101
  .long vector102
  .long vector103
  .long vector104
  .long vector105
  .long vector106
  .long vector107
  .long vector108
  .long vector109
  .long vector110
  .long vector111
  .long vector112
  .long vector113
  .long vector114
  .long vector115
  .long vector116
  .long vector117
  .long vector118
  .long vector119
  .long vector120
  .long vector121
  .long vector122
  .long vector123
  .long vector124
  .long vector125
  .long vector126
  .long vector127
  .long vector128
  .long vector129
  .long vector130
  .long vector131
  .long vector132
  .long vector133
  .long vector134
  .long vector135
  .long vector136
  .long vector137
  .long vector138
  .long vector139
  .long vector140
  .long vector141
  .long vector142
  .long vector143
  .long vector144
  .long vector145
  .long vector146
  .long vector147
  .long vector148
  .long vector149
  .long vector150
  .long vector151
  .long vector152
  .long vector153
  .long vector154
  .long vector155
  .long vector156
  .long vector157
  .long vector158
  .long vector159
  .long vector160
  .long vector161
  .long vector162
  .long vector163
  .long vector164
  .long vector165
  .long vector166
  .long vector167
  .long vector168
  .long vector169
  .long vector170
  .long vector171
  .long vector172
  .long vector173
  .long vector174
  .long vector175
  .long vector176
  .long vector177
  .long vector178
  .long vector179
  .long vector180
  .long vector181
  .long vector182
  .long vector183
  .long vector184
  .long vector185
  .long vector186
  .long vector187
  .long vector188
  .long vector189
  .long vector190
  .long vector191
  .long vector192
  .long vector193
  .long vector194
  .long vector195
  .long vector196
  .long vector197
  .long vector198
  .long vector199
  .long vector200
  .long vector201
  .long vector202
  .long vector203
  .long vector204
  .long vector205
  .long vector206
  .long vector207
  .long vector208
  .long vector209
  .long vector210
  .long vector211
  .long vector212
  .long vector213
  .long vector214
  .long vector215
  .long vector216
  .long vector217
  .long vector218
  .long vector219
  .long vector220
  .long vector221
  .long vector222
  .long vector223
  .long vector224
  .long vector225
  .long vector226
  .long vector227
  .long vector228
  .long vector229
  .long vector230
  .long vector231
  .long vector232
  .long vector233
  .long vector234
  .long vector235
  .long vector236
  .long vector237
  .long vector238
  .long vector239
  .long vector240
  .long vector241
  .long vector242
  .long vector243
  .long vector244
  .long vector245
  .long vector246
  .long vector247
  .long vector248
  .long vector249
  .long vector250
  .long vector251
  .long vector252
  .long vector253
  .long vector254
  .long vector255
