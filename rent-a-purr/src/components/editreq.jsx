import React, { useState } from 'react';
import { createPortal } from 'react-dom';
import { X, PawPrint, User, Phone, Calendar, Clock } from 'lucide-react';

const toInputDatetime = (dateString) => {
    if (!dateString) return '';
    return dateString.substring(0, 16).replace(' ', 'T');
};
const fromInputDatetime = (datetimeLocal) => datetimeLocal.replace('T', ' ') + ':00';

export default function EditReq({ cats, booking, onClose, onSubmit }) {
    // УМНАЯ ИНИЦИАЛИЗАЦИЯ: Ищем поля прямо в корне ответа бекенда (booking.phone)
    const[formData, setFormData] = useState({
        catId: booking.cat_id || booking.catId || '',
        customerName: booking.nickname || booking.customer?.name || '',
        customerPhone: booking.phone || booking.customer?.phone || '', // <--- ВОТ ГЛАВНОЕ ИСПРАВЛЕНИЕ
        customerEmail: booking.email || booking.customer?.email || '',
        start: toInputDatetime(booking.start_time || booking.time?.[0]),
        end: toInputDatetime(booking.end_time || booking.time?.[1])
    });

    const handleSubmit = (e) => {
        e.preventDefault();
        onSubmit({
            ...booking,
            // Возвращаем поля в том же виде, в котором они нужны бекенду (плоская структура)
            cat_id: Number(formData.catId),
            catId: Number(formData.catId), // для совместимости
            phone: formData.customerPhone,
            nickname: formData.customerName,
            email: formData.customerEmail,
            start_time: fromInputDatetime(formData.start),
            end_time: fromInputDatetime(formData.end),

            // На всякий случай старая структура для фронта
            customer: { name: formData.customerName, phone: formData.customerPhone, email: formData.customerEmail },
            time:[fromInputDatetime(formData.start), fromInputDatetime(formData.end)]
        });
    };

    return createPortal(
        <div className="fixed inset-0 z-[9999] flex items-center justify-center p-4 bg-slate-900/40 backdrop-blur-sm animate-in fade-in duration-200 font-m3">
            <div className="bg-white rounded-[32px] w-full max-w-2xl shadow-2xl border-2 border-[#7838F5]/20 overflow-hidden flex flex-col max-h-[90vh]">
                <div className="px-8 py-6 border-b-2 border-slate-100 flex justify-between items-center bg-white">
                    <h2 className="text-2xl font-extrabold text-slate-800 tracking-tight">Редактировать аренду</h2>
                    <button onClick={onClose} className="p-2 text-slate-400 hover:bg-slate-100 rounded-full transition-colors"><X size={24} strokeWidth={2.5} /></button>
                </div>
                <div className="p-8 overflow-y-auto no-scrollbar">
                    <form id="edit-req-form" onSubmit={handleSubmit} className="space-y-6">
                        <div className="space-y-2">
                            <label className="text-sm font-extrabold text-slate-700 ml-1 uppercase flex items-center gap-2"><PawPrint size={16} className="text-[#F6828C]" /> Выберите кота *</label>
                            <select required value={formData.catId} onChange={e => setFormData({...formData, catId: e.target.value})} className="w-full px-5 py-4 rounded-2xl bg-slate-50 border-2 border-slate-100 focus:border-[#7838F5] outline-none font-bold">
                                {cats.map(c => <option key={c.id} value={c.id}>{c.name} ({c.breed})</option>)}
                            </select>
                        </div>
                        <div className="grid grid-cols-1 md:grid-cols-2 gap-6">
                            <div className="space-y-2">
                                <label className="text-sm font-extrabold text-slate-700 ml-1 uppercase flex items-center gap-2"><User size={16} className="text-[#00C4D1]"/> Имя *</label>
                                <input required value={formData.customerName} onChange={e => setFormData({...formData, customerName: e.target.value})} className="w-full px-5 py-4 rounded-2xl bg-slate-50 border-2 border-slate-100 focus:border-[#7838F5] outline-none font-bold" />
                            </div>
                            <div className="space-y-2">
                                <label className="text-sm font-extrabold text-slate-700 ml-1 uppercase flex items-center gap-2"><Phone size={16} className="text-[#FF6B00]"/> Телефон *</label>
                                <input required value={formData.customerPhone} onChange={e => setFormData({...formData, customerPhone: e.target.value})} className="w-full px-5 py-4 rounded-2xl bg-slate-50 border-2 border-slate-100 focus:border-[#7838F5] outline-none font-bold" />
                            </div>
                        </div>
                        <div className="grid grid-cols-1 md:grid-cols-2 gap-6">
                            <div className="space-y-2">
                                <label className="text-sm font-extrabold text-slate-700 ml-1 uppercase flex items-center gap-2"><Calendar size={16} className="text-[#00D26A]"/> Старт *</label>
                                <input type="datetime-local" required value={formData.start} onChange={e => setFormData({...formData, start: e.target.value})} className="w-full px-5 py-4 rounded-2xl bg-slate-50 border-2 border-slate-100 focus:border-[#7838F5] outline-none font-bold" />
                            </div>
                            <div className="space-y-2">
                                <label className="text-sm font-extrabold text-slate-700 ml-1 uppercase flex items-center gap-2"><Clock size={16} className="text-[#FF2B4A]"/> Конец *</label>
                                <input type="datetime-local" required value={formData.end} onChange={e => setFormData({...formData, end: e.target.value})} className="w-full px-5 py-4 rounded-2xl bg-slate-50 border-2 border-slate-100 focus:border-[#7838F5] outline-none font-bold" />
                            </div>
                        </div>
                    </form>
                </div>
                <div className="px-8 py-5 border-t-2 border-slate-100 bg-white flex justify-end gap-3 rounded-b-[32px]">
                    <button type="button" onClick={onClose} className="px-6 py-3.5 rounded-2xl text-slate-500 font-extrabold hover:bg-slate-100 transition-colors">Отмена</button>
                    <button form="edit-req-form" type="submit" className="px-8 py-3.5 rounded-2xl bg-[#7838F5] hover:bg-[#6025D1] text-white font-extrabold shadow-lg shadow-[#7838F5]/30 transition-colors">Сохранить</button>
                </div>
            </div>
        </div>,
        document.body
    );
}