import React, { useState } from 'react';
import { createPortal } from 'react-dom';
import { X, PawPrint, User, Phone, Mail, Calendar, Clock } from 'lucide-react';

const fromInputDatetime = (datetimeLocal) => datetimeLocal.replace('T', ' ') + ':00';

export default function AddReq({ cats, onClose, onSubmit }) {
    const [formData, setFormData] = useState({ catId: cats[0]?.id || '', customerName: '', customerPhone: '', customerEmail: '', start: '', end: '' });

    const handleSubmit = (e) => {
        e.preventDefault();
        onSubmit({
            catId: Number(formData.catId),
            customer: { name: formData.customerName, phone: formData.customerPhone, email: formData.customerEmail },
            time:[fromInputDatetime(formData.start), fromInputDatetime(formData.end)]
        });
    };

    return createPortal(
        <div className="fixed inset-0 z-[9999] flex items-center justify-center p-4 bg-slate-900/40 backdrop-blur-sm animate-in fade-in duration-200 font-m3">
            <div className="bg-white rounded-[32px] w-full max-w-2xl shadow-2xl border-2 border-[#7838F5]/20 overflow-hidden flex flex-col max-h-[90vh]">
                <div className="px-8 py-6 border-b-2 border-slate-100 flex justify-between items-center bg-white">
                    <h2 className="text-2xl font-extrabold text-slate-800 tracking-tight">Новая аренда</h2>
                    <button onClick={onClose} className="p-2 text-slate-400 hover:bg-slate-100 rounded-full"><X size={24} strokeWidth={2.5} /></button>
                </div>
                <div className="p-8 overflow-y-auto no-scrollbar">
                    <form id="add-req-form" onSubmit={handleSubmit} className="space-y-6">
                        <div className="space-y-2"><label className="text-sm font-extrabold text-slate-700 ml-1 uppercase flex items-center gap-2"><PawPrint size={16} className="text-[#F6828C]" /> Выберите кота *</label><select required value={formData.catId} onChange={e => setFormData({...formData, catId: e.target.value})} className="w-full px-5 py-4 rounded-2xl bg-slate-50 border-2 border-slate-100 focus:border-[#7838F5] outline-none font-bold">{cats.map(c => <option key={c.id} value={c.id}>{c.name} ({c.breed})</option>)}</select></div>
                        <div className="grid grid-cols-1 md:grid-cols-2 gap-6">
                            <div className="space-y-2"><label className="text-sm font-extrabold text-slate-700 ml-1 uppercase flex items-center gap-2"><User size={16} className="text-[#00C4D1]"/> Имя *</label><input required value={formData.customerName} onChange={e => setFormData({...formData, customerName: e.target.value})} className="w-full px-5 py-4 rounded-2xl bg-slate-50 border-2 border-slate-100 focus:border-[#7838F5] outline-none font-bold" /></div>
                            <div className="space-y-2"><label className="text-sm font-extrabold text-slate-700 ml-1 uppercase flex items-center gap-2"><Phone size={16} className="text-[#FF6B00]"/> Телефон *</label><input required value={formData.customerPhone} onChange={e => setFormData({...formData, customerPhone: e.target.value})} className="w-full px-5 py-4 rounded-2xl bg-slate-50 border-2 border-slate-100 focus:border-[#7838F5] outline-none font-bold" /></div>
                        </div>
                        <div className="grid grid-cols-1 md:grid-cols-2 gap-6">
                            <div className="space-y-2"><label className="text-sm font-extrabold text-slate-700 ml-1 uppercase flex items-center gap-2"><Calendar size={16} className="text-[#00D26A]"/> Старт *</label><input type="datetime-local" required value={formData.start} onChange={e => setFormData({...formData, start: e.target.value})} className="w-full px-5 py-4 rounded-2xl bg-slate-50 border-2 border-slate-100 focus:border-[#7838F5] outline-none font-bold" /></div>
                            <div className="space-y-2"><label className="text-sm font-extrabold text-slate-700 ml-1 uppercase flex items-center gap-2"><Clock size={16} className="text-[#FF2B4A]"/> Конец *</label><input type="datetime-local" required value={formData.end} onChange={e => setFormData({...formData, end: e.target.value})} className="w-full px-5 py-4 rounded-2xl bg-slate-50 border-2 border-slate-100 focus:border-[#7838F5] outline-none font-bold" /></div>
                        </div>
                    </form>
                </div>
                <div className="px-8 py-5 border-t-2 border-slate-100 bg-white flex justify-end gap-3 rounded-b-[32px]">
                    <button onClick={onClose} className="px-6 py-3.5 rounded-2xl text-slate-500 font-extrabold hover:bg-slate-100">Отмена</button>
                    <button form="add-req-form" type="submit" className="px-8 py-3.5 rounded-2xl bg-[#7838F5] hover:bg-[#6025D1] text-white font-extrabold shadow-lg shadow-[#7838F5]/30">Сохранить</button>
                </div>
            </div>
        </div>,
        document.body
    );
}